#pragma once

#include "FW_FileParser.h"

class FW_XMLParser
{
public:
	FW_XMLParser(const char* aFile);

	bool BeginElement(const char* aElementName);
	void EndElement();

	int GetIntAttribute(const char* aAttributeName);
	const char* GetStringAttribute(const char* aAttributeName);

	void GetRawDataAsInt(FW_GrowingArray<int>& aRawDataOut, const char* aSeperator);

private:
	FW_FileParser myFileParser;
	FW_String myCurrentElement;

	// Sureley this is a nicer solution to this?
	// Is there more data that should be kept alive throughout nested elements?
	FW_GrowingArray<bool> myElementOnelinerStack;


	struct Attribute
	{
		FW_String myName;
		FW_String myStringValue;
	};
	FW_GrowingArray<Attribute> myAttributes;

	// Storage just used while parsing each element
	FW_GrowingArray<FW_String> myTempSplitElementStrings;
	FW_GrowingArray<FW_String> myTempAttributeStrings;
};
