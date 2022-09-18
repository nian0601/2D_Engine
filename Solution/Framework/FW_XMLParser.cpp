#include "FW_XMLParser.h"

FW_XMLParser::FW_XMLParser(const char* aFile)
	: myFileParser(aFile)
{
	myFileParser.ReadLine(myCurrentElement);
}

FW_XMLParser::FW_XMLParser(const FW_String& aFile)
	: FW_XMLParser(aFile.GetBuffer())
{
}

bool FW_XMLParser::BeginElement(const char* aElementName)
{
	if (myWasCurrentElementConsumed)
	{
		myAttributes.RemoveAll();

		myFileParser.ReadLine(myCurrentElement);
		myFileParser.TrimBeginAndEnd(myCurrentElement);

		myFileParser.SplitLineOnSpace(myCurrentElement, myTempSplitElementStrings);	
	}

	FW_String parsedElementName = myTempSplitElementStrings[0].SubStr(1, myTempSplitElementStrings[0].Length());
	if (parsedElementName[parsedElementName.Length()] == '>') // Surely there is a nicer solution to this?
		parsedElementName = parsedElementName.SubStr(0, parsedElementName.Length() - 1);

	if (parsedElementName != aElementName)
	{
		myWasCurrentElementConsumed = false;
		return false;
	}

	myWasCurrentElementConsumed = true;
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

	if (!myElementOnelinerStack.GetLast() && myWasCurrentElementConsumed)
		myFileParser.ReadLine(myCurrentElement);

	myElementOnelinerStack.RemoveLast();
	myWasCurrentElementConsumed = true;
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
