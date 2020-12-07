#pragma once

#include <Arduino.h>

#define MAX 1000

// Cuva poslednjih MAX vrednosti dobijenih sa LDR senzora u dva niza: originalne i normalizovane (uprosecene) vrednosti.
class LdrVals
{
private:
	// vrednosti dobijene sa LDR senzora
	int* vals;
	// normalizovane vrednosti dobijene sa LDR senzora
	int* valsAvg;
	// brojac 0, 1, ... MAX, 0, ... (ukrug)
	int cnt = 0;
    // prethodna originalna LDR vrednost
	int p;
    // pretprethodna originalna LDR vrednost
    int pp;

public:
	LdrVals();
	~LdrVals();

	// dodavanje nove LDR vrednosti
	int Add(int val);
	// prikazuje podatke oba niza
	//T void PrintAll();
    String PrintAll();

};

