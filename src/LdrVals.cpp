#include "LdrVals.h"
#include <Arduino.h>

LdrVals::LdrVals()
{
    vals = new int[MAX];
    valsAvg = new int[MAX];
}

LdrVals::~LdrVals()
{
    delete[] vals;
}

int LdrVals::Add(int val)
{
    vals[cnt] = val;

    int res = 0;
    if (cnt >= 2)
        res = (val + p + pp) / 3;
    else
    {
        if (cnt == 1)
            res = (val + p) / 2;
        if (cnt == 0)
            res = val;
    }
    pp = p;
    p = val;
    valsAvg[cnt++] = res;
    if (cnt >= MAX)
        cnt = 0;

    return res;
}

// Prikaz svih elemenata u izvornom i normalizovanom nizu.
// Radi samo ako su nizovi potpuno popunjeni.
// void LdrVals::PrintAll()
// {
// 	// od najstarijeg elementa do kraja niza
// 	for (int j = cnt; j < MAX; j++)
// 		cout << vals[j] << '\t' << valsAvg[j] << '\n';
// 	// od pocetka niza do najmladjeg elementa
// 	for (int j = 0; j < cnt; j++)
// 		cout << vals[j] << '\t' << valsAvg[j] << '\n';
// }

String LdrVals::PrintAll()
{
    String s;
    for (int j = cnt; j < MAX; j++)
    {
        s += vals[j];
        s += '\t';
        s += valsAvg[j];
        s += '\n';
    }
    for (int j = 0; j < cnt; j++)
    {
        s += vals[j];
        s += '\t';
        s += valsAvg[j];
        s += '\n';
    }
    return s;
}