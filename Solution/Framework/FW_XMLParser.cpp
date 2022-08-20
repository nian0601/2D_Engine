#include "FW_XMLParser.h"

FW_XMLParser::FW_XMLParser(const char* aFile)
	: myFileParser(aFile)
{
	myFileParser.ReadLine(myCurrentElement);
}

bool FW_XMLParser::BeginElement(const char* aElementName)
{
	myAttributes.RemoveAll();

	myFileParser.ReadLine(myCurrentElement);
	myFileParser.TrimBeginAndEnd(myCurrentElement);

	myFileParser.SplitLineOnSpace(myCurrentElement, myTempSplitElementStrings);

	FW_String parsedElementName = myTempSplitElementStrings[0].SubStr(1, myTempSplitElementStrings[0].Length());
	if (parsedElementName != aElementName)
		return false;

	myTempSplitElementStrings.RemoveNonCyclicAtIndex(0);

	bool currentElementIsOneLine = false;
	
	for (const FW_String& attributeString : myTempSplitElementStrings)
	{
		myFileParser.SplitLine(attributeString, "=", myTempAttributeStrings);

		Attribute& attribute = myAttributes.Add();
		attribute.myName = myTempAttributeStrings[0];
		attribute.myStringValue = myTempAttributeStrings[1];

		// Clean this up, unnecessary duplication
		if (attribute.myStringValue[attribute.myStringValue.Length() - 1] == '/')
		{
			attribute.myStringValue = attribute.myStringValue.SubStr(1, attribute.myStringValue.Length() - 3);
			currentElementIsOneLine = true;
		}
		else
		{
			attribute.myStringValue = attribute.myStringValue.SubStr(1, attribute.myStringValue.Length() - 1);
		}
	}

	myElementOnelinerStack.Add(currentElementIsOneLine);

	return true;
}

void FW_XMLParser::EndElement()
{
	FW_ASSERT(!myElementOnelinerStack.IsEmpty(), "Tried to end 'EndElement' before successfully calling 'BeingElement'");

	if(!myElementOnelinerStack.GetLast())
		myFileParser.ReadLine(myCurrentElement);

	myElementOnelinerStack.RemoveLast();
}

int FW_XMLParser::GetIntAttribute(const char* aAttributeName)
{
	for (const Attribute& attribute : myAttributes)
	{
		if (attribute.myName == aAttributeName)
			return atoi(attribute.myStringValue.GetBuffer());
	}

	return 0;
}

const char* FW_XMLParser::GetStringAttribute(const char* aAttributeName)
{
	for (const Attribute& attribute : myAttributes)
	{
		if (attribute.myName == aAttributeName)
			return attribute.myStringValue.GetBuffer();
	}

	return nullptr;
}

void FW_XMLParser::GetRawDataAsInt(FW_GrowingArray<int>& aRawDataOut, const char* aSeperator)
{
	aRawDataOut.RemoveAll();

	myFileParser.ReadLine(myCurrentElement);
	myFileParser.TrimBeginAndEnd(myCurrentElement);

	myFileParser.SplitLine(myCurrentElement, aSeperator, myTempSplitElementStrings);
	for (const FW_String& string : myTempSplitElementStrings)
		aRawDataOut.Add(atoi(string.GetBuffer()));
}
