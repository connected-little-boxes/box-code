#include <Arduino.h>
#include <limits.h>
#include "utils.h"

#define LED_BUILTIN 2

int rand_seed=1234;
int rand_mult=8121;
int rand_add=28411;
int rand_modulus=134456;

int localRand()
{
  rand_seed = (rand_mult * rand_seed + rand_add) % rand_modulus;
  return rand_seed;
}

int localRand(int limit)
{
    return localRand() % limit;
}

void localSrand(int seed)
{    
    rand_seed = seed;
}

int localRand(int low, int high)
{
    int diff = high - low;
    return localRand(diff) + low;
}

unsigned long ulongDiff(unsigned long end, unsigned long start)
{
    if (end >= start)
    {
        return end - start;
    }
    else
    {
        return ULONG_MAX - start + end + 1;
    }
}

bool strContains(char *searchMe, char *findMe)
{
    while (*searchMe != 0)
    {
        if (*searchMe == *findMe)
        {
            // found the start of the find string
            // try to match the rest of the find string from here
            char *s1 = searchMe;
            char *f1 = findMe;
            while (*s1 == *f1)
            {
                f1++;
                s1++;
                if (*f1 == 0)
                {
                    // hit the end of the find string - return with a win
                    return true;
                }
                if (*s1 == 0)
                {
                    // hit the end of the search string before we
                    // completed the find - return with a fail
                    return false;
                }
            }
        }
        searchMe++;
    }
    return false;
}

int getUnalignedInt(unsigned char *source)
{
    int result;
    memcpy((unsigned char *)&result, source, sizeof(int));
    return result;
}

float getUnalignedFloat(unsigned char *source)
{
    float result;
    memcpy((unsigned char *)&result, source, sizeof(float));
    return result;
}

void putUnalignedFloat(float fval, unsigned char *dest)
{
    unsigned char *source = (unsigned char *)&fval;
    memcpy(dest, source, sizeof(float));
}

float getUnalignedDouble(unsigned char *source)
{
    double result;
    memcpy((unsigned char *)&result, source, sizeof(double));
    return result;
}

void putUnalignedDouble(double dval, unsigned char *dest)
{
    unsigned char *source = (unsigned char *)&dval;
    memcpy(dest, source, sizeof(double));
}
