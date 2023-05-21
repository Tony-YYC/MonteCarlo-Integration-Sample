#include <random>
#include <time.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;
class Integration
{
public:
    int num_samples;
    double upper_limit;
    double lower_limit;
    double f(double x);                              // 积分函数
    Integration(double up, double low, int samples); // 构造函数
    double getresultNum();                           // 普通数值法
    double getresultMC();                            // 蒙特卡洛积分法，采用均匀分布采样
    double getresultMCAR();                          // 蒙特卡洛积分法，采用拒绝-接受采样
    double getresultMCMH();                          // 蒙特卡洛积分法，采用MH采样（马尔科夫链）
    void MHSampling();                               // MH采样实现
    void ARSampling();                               // 拒绝-接受采样实现
    void writeFile(string filename);

private:
    double targetDistribution(double x); // 采样目标分布
    double *sample;                      // 采样样本数组
    double *section;                     // 预抽样决定采样的目标分布参数
    int sections;                        // 预抽样样本数
    void PreSampling();                  // 预抽样过程，生成目标分布
    double min(double a, double b);      // 取最小值的实现
};

Integration::Integration(double up, double low, int samples)
{ // 类的构造函数
    num_samples = samples;
    upper_limit = up;
    lower_limit = low;
    sections = 20;
}
double Integration::f(double x)
{ // 积分函数定义
    return x * x;
}
double Integration::getresultNum()
{ // 普通数值法实现
    double step = (upper_limit - lower_limit) / num_samples;
    double result = 0;
    for (double i = lower_limit; i <= upper_limit; i += step)
    {
        result += f(i) * step;
    }
    return result;
}

double Integration::getresultMC()
{ // 蒙特卡洛法实现
    double result = 0;
    std::random_device rd;
    std::default_random_engine seed(rd());
    std::uniform_real_distribution<double> random(lower_limit, upper_limit);
    for (int i = 0; i < num_samples; i++)
    {
        double sample = random(seed);
        result += f(sample) * (upper_limit - lower_limit) / num_samples;
    }
    return result;
}

double Integration::targetDistribution(double x)
{ // 目标分布函数
    double step = (upper_limit - lower_limit) / sections;
    int sectionNum = floor(x / step);
    return *(section + sectionNum);
}

double Integration::min(double a, double b)
{ // 最小值实现
    return a < b ? a : b;
}

void Integration::writeFile(string filename)
{
    std::ofstream p;
    p.open(filename, std::ios::out | std::ios::trunc);
    p.precision(16);
    for (int i = 0; i < num_samples; i++)
    {
        p << sample[i] << std::endl;
    }
    p.close();
}

void Integration::PreSampling()
{ // 预抽样实现
    section = new double[sections];
    double sectionSum = 0;
    double step = (upper_limit - lower_limit) / sections;
    for (int i = 0; i < sections; i++) // 计算预抽样样本
    {
        double sectionValue = (f(lower_limit + step * i) + f(lower_limit + step * (i + 1)) / 2);
        sectionSum += sectionValue;
        section[i] = sectionValue;
    }
    for (int i = 0; i < sections; i++) // 归一化
    {
        section[i] /= (sectionSum * step);
        // std::cout << section[i] << std::endl;
    }
}

void Integration::MHSampling()
{
    PreSampling();
    sample = new double[num_samples];
    for (int i = 0; i < num_samples; i++)
    {
        sample[i] = 0;
    }
    std::random_device rd;
    std::default_random_engine seed(rd());
    std::uniform_real_distribution<double> random(lower_limit, upper_limit);
    std::uniform_real_distribution<double> random1(0, 1);
    for (int i = 0; i < num_samples; i++)
    {
        double candidateValue = random(seed);
        double currentValue;
        currentValue = i > 0 ? sample[i-1] : sample[i];
        double alpha = min(1, targetDistribution(candidateValue) / targetDistribution(currentValue));

        double u = random1(seed);
        if (u < alpha)
        {
            currentValue = candidateValue;
        }
        sample[i] = currentValue;
    }
}

void Integration::ARSampling()
{
    PreSampling();
    double c = 3.0;
    sample = new double[num_samples];
    std::random_device rd;
    std::default_random_engine seed(rd());
    std::uniform_real_distribution<double> random(lower_limit, upper_limit);
    std::uniform_real_distribution<double> random1(0, 1);
    double candidateValue, alpha, u;
    for (int i = 0; i < num_samples; i++)
    {
        do
        {
            candidateValue = random(seed);
            alpha = targetDistribution(candidateValue) / (c * (1 / (upper_limit - lower_limit)));
            u = random1(seed);
        } while (u >= alpha);
        sample[i] = candidateValue;
    }
}

double Integration::getresultMCMH()
{
    double result = 0;
    MHSampling();
    for (int i = 0; i < num_samples; i++)
    {
        result += f(sample[i]) / num_samples / targetDistribution(sample[i]);
    }
    return result;
}

double Integration::getresultMCAR()
{
    double result = 0;
    ARSampling();
    for (int i = 0; i < num_samples; i++)
    {
        result += f(sample[i]) / num_samples / targetDistribution(sample[i]);
    }
    return result;
}

double Variance(const vector<double>& data)
{
    double sum = std::accumulate(std::begin(data), std::end(data), 0.0);
    double mean = sum / data.size();

    double variance = 0.0;
    std::for_each(std::begin(data), std::end(data), [&](const double d) {
        variance += pow(d-mean, 2);
    });
    variance /= data.size();

    return variance;
}

int main()
{
    
    Integration integration(1, 0, 1000);
    //integration.MHSampling();
    //integration.writeFile("outputMHsampling.csv");
    
    /* //方差计算
    std::ofstream p;
    p.open("output.csv", std::ios::out | std::ios::trunc);
    p.precision(16);
    for (int i = 10; i <= 500; i++)
    {
        Integration integration(1, 0, i);
        vector<double> tempMC,tempMCMH,tempMCAR;
        for(int j = 1;j<=100;j++){
            tempMC.push_back(integration.getresultMC());
            tempMCMH.push_back(integration.getresultMCMH());
            tempMCAR.push_back(integration.getresultMCAR());
        }
        p << Variance(tempMC) << ",";
        p << Variance(tempMCMH) << ",";
        p << Variance(tempMCAR) << ",";
        p << i << std::endl;
        cout << i << "completed" << std::endl;
    }
    p.close();
    */
    std::ofstream p;
    p.open("output.csv", std::ios::out | std::ios::trunc);
    p.precision(16);
    for (int i = 0; i <= 1000; i++)
    {
        p << integration.getresultMC() << ",";
        p << integration.getresultMCMH() << ",";
        p << integration.getresultMCAR() << std::endl;
        cout << i << "completed" << std::endl;
    }
    p.close();
    return 0;
}