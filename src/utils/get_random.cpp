#include <get_random.h>
#include <types.h>

namespace solstice
{

std::random_device Random::rd;

String Random::getRandomUid()
{
    static std::mt19937_64 rng(std::random_device{}());
    static std::uniform_int_distribution<uint64_t> dist;
    String uid = std::to_string(dist(rng));

    uid.insert(uid.begin(), 20 - uid.length(), '0');

    return uid;
}

int Random::getRandomInt(int min, int max)
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(min, max);

    return dist(gen);
}

double Random::getRandomDouble(double min, double max)
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dist(min, max);

    double value = dist(gen);

    return std::round(value * 100.0) / 100.0;
}

int Random::getRandomBool()
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, 1);

    return dist(gen) == 1;
}

}  // namespace solstice
