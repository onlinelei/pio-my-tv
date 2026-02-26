#ifndef TFT_SINGLETON_H
#define TFT_SINGLETON_H

#include <TFT_eSPI.h>

class TFT_Singleton
{
public:
    static TFT_Singleton &getInstance()
    {
        static TFT_Singleton instance;
        return instance;
    }

    TFT_eSPI &getTFT()
    {
        return tft;
    }

private:
    TFT_Singleton()
    {
        tft.begin();        /* TFT init */
        tft.setRotation(0); /* Landscape orientation, flipped */
    }

    ~TFT_Singleton() = default;

    TFT_Singleton(const TFT_Singleton &) = delete;
    TFT_Singleton &operator=(const TFT_Singleton &) = delete;

    TFT_eSPI tft;
};

#endif // TFT_SINGLETON_H
