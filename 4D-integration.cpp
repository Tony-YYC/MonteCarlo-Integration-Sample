#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <random>
#include <fstream>


struct Vec
{
    // 使用C++的运算符重载来实现向量运算，简化后面的运算过程
    double x, y, z, w;

    inline Vec operator+(Vec const &r) const { return {x + r.x, y + r.y, z + r.z, w + r.w}; }
    inline Vec operator-(Vec const &r) const { return {x - r.x, y - r.y, z - r.z, w - r.w}; }
    inline Vec operator*(double const &s) const { return {x * s, y * s, z * s, w * s}; }
    inline Vec operator/(double const &s) const { return {x / s, y / s, z / s, w / s}; }
    inline double volume() const { return x * y * z * w; }
};

class Integration
{
public:
    int num_samples;                             // 单方向上的采样数量
    Vec *upper_limit;                            // 积分区域的起始点
    Vec *lower_limit;                            // 积分区域的终止点
    double f(Vec *inputVector);                  // 积分函数
    Integration(Vec *up, Vec *low, int samples); // 构造函数
    double getresultNum();                       // 普通数值法
    double getresultMC();                        // 蒙特卡洛积分法
private:
    Vec randomGenerator();
};

Integration::Integration(Vec *up, Vec *low, int samples)
{
    num_samples = samples;
    upper_limit = up;
    lower_limit = low;
}

double Integration::f(Vec *inputVector)
{ // 积分函数定义
    return pow(inputVector->x, 2) + pow(inputVector->y, 2) + pow(inputVector->z, 2) + pow(inputVector->w, 2);
}

double Integration::getresultNum()
{ // 普通数值法实现
    Vec step = ((*upper_limit) - (*lower_limit)) / num_samples;
    double result = 0;
    Vec temp = * lower_limit;
    for (temp.x = lower_limit->x; temp.x <= upper_limit->x; temp.x += step.x)
    {
        for (temp.y = lower_limit->y; temp.y <= upper_limit->y; temp.y += step.y)
        {
            for (temp.z = lower_limit->z; temp.z <= upper_limit->z; temp.z += step.z)
            {
                for (temp.w = lower_limit->w; temp.w <= upper_limit->w; temp.w += step.w)
                {
                    Vec input = temp + step / 2;
                    result += step.volume() * f(&input);
                }
            }
        }
    }
    return result;
}

Vec Integration::randomGenerator()
{
    Vec randomVec;
    std::random_device rd;
    std::default_random_engine seed(rd());
    std::uniform_real_distribution<double> random(lower_limit->x, upper_limit->x);
    randomVec.x = random(seed);
    std::uniform_real_distribution<double> random1(lower_limit->y, upper_limit->y);
    randomVec.y = random1(seed);
    std::uniform_real_distribution<double> random2(lower_limit->z, upper_limit->z);
    randomVec.z = random2(seed);
    std::uniform_real_distribution<double> random3(lower_limit->w, upper_limit->w);
    randomVec.w = random3(seed);
    return randomVec;
}
double Integration::getresultMC()
{ // 蒙特卡洛法实现
    double result = 0;
    int up = pow(num_samples, 4);
    Vec sample;
    for (int i = 0; i < up; i++)
    {
        sample = randomGenerator();
        result += f(&sample) * ((*upper_limit - *lower_limit).volume()) / pow(num_samples, 4);
    }
    return result;
}

int main()
{
    Vec up = {1, 1, 1, 1};
    Vec low = {0, 0, 0, 0};
    //Integration integration(&up, &low, 10);
    std::ofstream p;
    p.open("output.csv",std::ios::out|std::ios::trunc); 
    p.precision(16);
    for(int i = 10;i<=60;i++){
        Integration integration(&up, &low , i);
        p << integration.getresultMC() << ",";
        p << integration.getresultNum() << std::endl;
        std::cout << i << "completed" << std::endl;
    }
    p.close();
    return 0;
}