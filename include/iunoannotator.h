#pragma once

#include <vector>
#include <stdint.h>

typedef enum
{
	AnnotatorStyleFlag = 1,
	AnnotatorStyleBox = 2,
	AnnotatorStyleMarker = 3,
	AnnotatorStyleMarkerAndLine = 4
} IUnoAnnotatorStyle;

typedef struct
{
	long long frequency;
	int power;
	std::string text;
	IUnoAnnotatorStyle style;
	uint32_t rgb;
	long long lineToFreq;
	int lineToPower;
} IUnoAnnotatorItem;

class IUnoAnnotator
{

public:

	virtual void AnnotatorProcess(std::vector<IUnoAnnotatorItem>& items) = 0;
};
