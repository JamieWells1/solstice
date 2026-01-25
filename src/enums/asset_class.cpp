#include <asset_class.h>
#include <types.h>

#include <ostream>
#include <random>

namespace solstice
{

// ===== AssetClass ====

const char* to_string(AssetClass cls) { return ASSET_CLASS_STR[static_cast<size_t>(cls)]; }

std::ostream& operator<<(std::ostream& os, AssetClass assetClass)
{
    return os << to_string(assetClass);
}

AssetClass randomAssetClass()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(AssetClass::COUNT) - 1);

    return static_cast<AssetClass>(dist(gen));
}

Resolution<Underlying> getUnderlying(AssetClass assetClass)
{
    switch (assetClass)
    {
        case AssetClass::Equity:
        {
            auto underlying = randomUnderlying<Equity>();
            if (!underlying)
            {
                return resolution::err(underlying.error());
            }
            return Underlying(*underlying);
        }
        case AssetClass::Future:
        {
            auto underlying = randomUnderlying<Future>();
            if (!underlying)
            {
                return resolution::err(underlying.error());
            }
            return Underlying(*underlying);
        }
        case AssetClass::Option:
        {
            auto underlying = randomUnderlying<Option>();
            if (!underlying)
            {
                return resolution::err(underlying.error());
            }
            return Underlying(*underlying);
        }
        default:
            return resolution::err("Invalid asset class\n");
    }
}

// template definitions moved to asset_class.h

}  // namespace solstice
