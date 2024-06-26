/******************************************************************************************/
/**** FILE: AntennaModelMap.cpp                                                        ****/
/******************************************************************************************/

#include <limits>
#include <algorithm>
#include "AntennaModelMap.h"

/******************************************************************************************/
/**** CONSTRUCTOR: AntennaModelClass::AntennaModelClass                                ****/
/******************************************************************************************/
AntennaModelClass::AntennaModelClass(std::string nameVal) : name(nameVal)
{
	type = AntennaModel::UnknownType;
	category = AntennaModel::UnknownCategory;
	diameterM = -1.0;
	midbandGain = std::numeric_limits<double>::quiet_NaN();
	reflectorWidthM = -1.0;
	reflectorHeightM = -1.0;
}
/******************************************************************************************/

/******************************************************************************************/
/**** CONSTRUCTOR: AntennaPrefixClass::AntennaPrefixClass                              ****/
/******************************************************************************************/
AntennaPrefixClass::AntennaPrefixClass(std::string prefixVal) : prefix(prefixVal)
{
	type = AntennaModel::UnknownType;
	category = AntennaModel::UnknownCategory;
}
/******************************************************************************************/

/******************************************************************************************/
/**** STATIC FUNCTION: AntennaModel::categoryStr()                                     ****/
/******************************************************************************************/
std::string AntennaModel::categoryStr(AntennaModel::CategoryEnum categoryVal)
{
	std::string str;

	switch (categoryVal) {
		case B1Category:
			str = "B1";
			break;
		case HPCategory:
			str = "HP";
			break;
		case OtherCategory:
			str = "OTHER";
			break;
		case UnknownCategory:
			str = "UNKNOWN";
			break;
		default:
			CORE_DUMP;
			break;
	}

	return (str);
}
/******************************************************************************************/

/******************************************************************************************/
/**** STATIC FUNCTION: AntennaModel::typeStr()                                         ****/
/******************************************************************************************/
std::string AntennaModel::typeStr(AntennaModel::TypeEnum typeVal)
{
	std::string str;

	switch (typeVal) {
		case AntennaType:
			str = "Ant";
			break;
		case ReflectorType:
			str = "Ref";
			break;
		case UnknownType:
			str = "UNKNOWN";
			break;
		default:
			CORE_DUMP;
			break;
	}

	return (str);
}
/******************************************************************************************/

/******************************************************************************************/
/**** CONSTRUCTOR: AntennaModelMapClass::AntennaModelMapClass                          ****/
/******************************************************************************************/
AntennaModelMapClass::AntennaModelMapClass(std::string antModelListFile,
					   std::string antPrefixListFile,
					   std::string antModelMapFile)
{
	readModelList(antModelListFile);
	readPrefixList(antPrefixListFile);
	readModelMap(antModelMapFile);
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: AntennaModelMapClass::readModelList()                                  ****/
/******************************************************************************************/
void AntennaModelMapClass::readModelList(const std::string filename)
{
	int linenum, fIdx;
	std::string line, strval;
	char *chptr;
	FILE *fp = (FILE *)NULL;
	std::string str;
	std::string reasonIgnored;
	std::ostringstream errStr;

	int modelNameFieldIdx = -1;
	int typeFieldIdx = -1;
	int categoryFieldIdx = -1;
	int diameterMFieldIdx = -1;
	int midbandGainFieldIdx = -1;
	int reflectorWidthMFieldIdx = -1;
	int reflectorHeightMFieldIdx = -1;

	std::vector<int *> fieldIdxList;
	std::vector<std::string> fieldLabelList;
	fieldIdxList.push_back(&modelNameFieldIdx);
	fieldLabelList.push_back("Ant Model");
	fieldIdxList.push_back(&typeFieldIdx);
	fieldLabelList.push_back("Type");
	fieldIdxList.push_back(&categoryFieldIdx);
	fieldLabelList.push_back("Category");
	fieldIdxList.push_back(&diameterMFieldIdx);
	fieldLabelList.push_back("Diameter (m)");
	fieldIdxList.push_back(&midbandGainFieldIdx);
	fieldLabelList.push_back("Midband Gain (dBi)");
	fieldIdxList.push_back(&reflectorWidthMFieldIdx);
	fieldLabelList.push_back("Reflector Width (m)");
	fieldIdxList.push_back(&reflectorHeightMFieldIdx);
	fieldLabelList.push_back("Reflector Height (m)");

	std::string name;
	AntennaModel::CategoryEnum category;
	AntennaModel::TypeEnum type;
	double diameterM;
	double midbandGain;
	double reflectorWidthM = -1;
	double reflectorHeightM = -1;

	int fieldIdx;

	if (filename.empty()) {
		throw std::runtime_error("ERROR: No Antenna Model List File specified");
	}

	if (!(fp = fopen(filename.c_str(), "rb"))) {
		str = std::string("ERROR: Unable to open Antenna Model List File \"") + filename +
		      std::string("\"\n");
		throw std::runtime_error(str);
	}

	enum LineTypeEnum { labelLineType, dataLineType, ignoreLineType, unknownLineType };

	LineTypeEnum lineType;

	AntennaModelClass *antennaModel;

	linenum = 0;
	bool foundLabelLine = false;
	while (fgetline(fp, line, false)) {
		linenum++;
		std::vector<std::string> fieldList = splitCSV(line);

		lineType = unknownLineType;
		/**************************************************************************/
		/**** Determine line type                                              ****/
		/**************************************************************************/
		if (fieldList.size() == 0) {
			lineType = ignoreLineType;
		} else {
			fIdx = fieldList[0].find_first_not_of(' ');
			if (fIdx == (int)std::string::npos) {
				if (fieldList.size() == 1) {
					lineType = ignoreLineType;
				}
			} else {
				if (fieldList[0].at(fIdx) == '#') {
					lineType = ignoreLineType;
				}
			}
		}

		if ((lineType == unknownLineType) && (!foundLabelLine)) {
			lineType = labelLineType;
			foundLabelLine = 1;
		}
		if ((lineType == unknownLineType) && (foundLabelLine)) {
			lineType = dataLineType;
		}
		/**************************************************************************/

		/**************************************************************************/
		/**** Process Line                                                     ****/
		/**************************************************************************/
		bool found;
		std::string field;
		switch (lineType) {
			case labelLineType:
				for (fieldIdx = 0; fieldIdx < (int)fieldList.size(); fieldIdx++) {
					field = fieldList.at(fieldIdx);

					// std::cout << "FIELD: \"" << field << "\"" << std::endl;

					found = false;
					for (fIdx = 0;
					     (fIdx < (int)fieldLabelList.size()) && (!found);
					     fIdx++) {
						if (field == fieldLabelList.at(fIdx)) {
							*fieldIdxList.at(fIdx) = fieldIdx;
							found = true;
						}
					}
				}

				for (fIdx = 0; fIdx < (int)fieldIdxList.size(); fIdx++) {
					if (*fieldIdxList.at(fIdx) == -1) {
						errStr << "ERROR: Invalid Antenna Model List file "
							  "\""
						       << filename << "\" label line missing \""
						       << fieldLabelList.at(fIdx) << "\""
						       << std::endl;
						throw std::runtime_error(errStr.str());
					}
				}

				break;
			case dataLineType:
				/**************************************************************************/
				/* modelName */
				/**************************************************************************/
				strval = fieldList.at(modelNameFieldIdx);
				if (strval.empty()) {
					errStr << "ERROR: Antenna Model List file \"" << filename
					       << "\" line " << linenum << " missing model name"
					       << std::endl;
					throw std::runtime_error(errStr.str());
				}

				name = strval;
				/**************************************************************************/

				/**************************************************************************/
				/* type */
				/**************************************************************************/
				strval = fieldList.at(typeFieldIdx);
				if (strval.empty()) {
					errStr << "ERROR: Antenna Model List file \"" << filename
					       << "\" line " << linenum << " missing type"
					       << std::endl;
					throw std::runtime_error(errStr.str());
				}

				if ((strval == "Ant") || (strval == "Antenna")) {
					type = AntennaModel::AntennaType;
				} else if ((strval == "Ref") || (strval == "Reflector")) {
					type = AntennaModel::ReflectorType;
				} else {
					errStr << "ERROR: Antenna Model List file \"" << filename
					       << "\" line " << linenum
					       << " invalid type: " << strval << std::endl;
					throw std::runtime_error(errStr.str());
				}
				/**************************************************************************/

				/**************************************************************************/
				/* category */
				/**************************************************************************/
				strval = fieldList.at(categoryFieldIdx);
				if (strval.empty()) {
					errStr << "ERROR: Antenna Model List file \"" << filename
					       << "\" line " << linenum << " missing category"
					       << std::endl;
					throw std::runtime_error(errStr.str());
				}

				if (strval == "HP") {
					category = AntennaModel::HPCategory;
				} else if (strval == "B1") {
					category = AntennaModel::B1Category;
				} else if ((strval == "OTHER") || (strval == "Other")) {
					category = AntennaModel::OtherCategory;
				} else {
					errStr << "ERROR: Antenna Model List file \"" << filename
					       << "\" line " << linenum
					       << " invalid category: " << strval << std::endl;
					throw std::runtime_error(errStr.str());
				}
				/**************************************************************************/

				/**************************************************************************/
				/* diameter */
				/**************************************************************************/
				strval = fieldList.at(diameterMFieldIdx);
				if (strval.empty()) {
					diameterM = -1.0; // Use -1 for unknown
				} else {
					diameterM = std::strtod(strval.c_str(), &chptr);
					if (diameterM <= 0.0) {
						errStr << "ERROR: Antenna Model List file \""
						       << filename << "\" line " << linenum
						       << " invalid diameter: \"" << strval << "\""
						       << std::endl;
						throw std::runtime_error(errStr.str());
					}
					// Use meters in input file, no conversion here
					// diameter *= 12*2.54*0.01; // convert ft to meters
				}
				/**************************************************************************/

				/**************************************************************************/
				/* midband gain */
				/**************************************************************************/
				strval = fieldList.at(midbandGainFieldIdx);
				if (strval.empty()) {
					midbandGain = std::numeric_limits<double>::quiet_NaN();
				} else {
					midbandGain = std::strtod(strval.c_str(), &chptr);
				}
				/**************************************************************************/

				/**************************************************************************/
				/* reflectorWidth */
				/**************************************************************************/
				strval = fieldList.at(reflectorWidthMFieldIdx);
				if (strval.empty()) {
					reflectorWidthM = -1.0; // Use -1 for unknown
				} else {
					reflectorWidthM = std::strtod(strval.c_str(), &chptr);
					if (reflectorWidthM <= 0.0) {
						errStr << "ERROR: Antenna Model List file \""
						       << filename << "\" line " << linenum
						       << " invalid reflector width: \"" << strval
						       << "\"" << std::endl;
						throw std::runtime_error(errStr.str());
					}
				}
				/**************************************************************************/

				/**************************************************************************/
				/* reflectorHeight */
				/**************************************************************************/
				strval = fieldList.at(reflectorHeightMFieldIdx);
				if (strval.empty()) {
					reflectorHeightM = -1.0; // Use -1 for unknown
				} else {
					reflectorHeightM = std::strtod(strval.c_str(), &chptr);
					if (reflectorHeightM <= 0.0) {
						errStr << "ERROR: Antenna Model List file \""
						       << filename << "\" line " << linenum
						       << " invalid reflector height: \"" << strval
						       << "\"" << std::endl;
						throw std::runtime_error(errStr.str());
					}
				}
				/**************************************************************************/

				antennaModel = new AntennaModelClass(name);
				antennaModel->setCategory(category);
				antennaModel->setType(type);
				antennaModel->setDiameterM(diameterM);
				antennaModel->setMidbandGain(midbandGain);
				antennaModel->setReflectorWidthM(reflectorWidthM);
				antennaModel->setReflectorHeightM(reflectorHeightM);

				antennaModelList.push_back(antennaModel);

				break;
			case ignoreLineType:
			case unknownLineType:
				// do nothing
				break;
			default:
				CORE_DUMP;
				break;
		}
	}

	if (fp) {
		fclose(fp);
	}

	return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: AntennaModelMapClass::readPrefixList()                                 ****/
/******************************************************************************************/
void AntennaModelMapClass::readPrefixList(const std::string filename)
{
	int linenum, fIdx;
	std::string line, strval;
	FILE *fp = (FILE *)NULL;
	std::string str;
	std::string reasonIgnored;
	std::ostringstream errStr;

	int prefixFieldIdx = -1;
	int typeFieldIdx = -1;
	int categoryFieldIdx = -1;

	std::vector<int *> fieldIdxList;
	std::vector<std::string> fieldLabelList;
	fieldIdxList.push_back(&prefixFieldIdx);
	fieldLabelList.push_back("Prefix");
	fieldIdxList.push_back(&typeFieldIdx);
	fieldLabelList.push_back("Type");
	fieldIdxList.push_back(&categoryFieldIdx);
	fieldLabelList.push_back("Category");

	std::string prefix;
	AntennaModel::CategoryEnum category;
	AntennaModel::TypeEnum type;

	int fieldIdx;

	if (filename.empty()) {
		throw std::runtime_error("ERROR: No Antenna Prefix File specified");
	}

	if (!(fp = fopen(filename.c_str(), "rb"))) {
		str = std::string("ERROR: Unable to open Antenna Prefix File \"") + filename +
		      std::string("\"\n");
		throw std::runtime_error(str);
	}

	enum LineTypeEnum { labelLineType, dataLineType, ignoreLineType, unknownLineType };

	LineTypeEnum lineType;

	AntennaPrefixClass *antennaPrefix;

	linenum = 0;
	bool foundLabelLine = false;
	while (fgetline(fp, line, false)) {
		linenum++;
		std::vector<std::string> fieldList = splitCSV(line);

		lineType = unknownLineType;
		/**************************************************************************/
		/**** Determine line type                                              ****/
		/**************************************************************************/
		if (fieldList.size() == 0) {
			lineType = ignoreLineType;
		} else {
			fIdx = fieldList[0].find_first_not_of(' ');
			if (fIdx == (int)std::string::npos) {
				if (fieldList.size() == 1) {
					lineType = ignoreLineType;
				}
			} else {
				if (fieldList[0].at(fIdx) == '#') {
					lineType = ignoreLineType;
				}
			}
		}

		if ((lineType == unknownLineType) && (!foundLabelLine)) {
			lineType = labelLineType;
			foundLabelLine = 1;
		}
		if ((lineType == unknownLineType) && (foundLabelLine)) {
			lineType = dataLineType;
		}
		/**************************************************************************/

		/**************************************************************************/
		/**** Process Line                                                     ****/
		/**************************************************************************/
		bool found;
		std::string field;
		switch (lineType) {
			case labelLineType:
				for (fieldIdx = 0; fieldIdx < (int)fieldList.size(); fieldIdx++) {
					field = fieldList.at(fieldIdx);

					// std::cout << "FIELD: \"" << field << "\"" << std::endl;

					found = false;
					for (fIdx = 0;
					     (fIdx < (int)fieldLabelList.size()) && (!found);
					     fIdx++) {
						if (field == fieldLabelList.at(fIdx)) {
							*fieldIdxList.at(fIdx) = fieldIdx;
							found = true;
						}
					}
				}

				for (fIdx = 0; fIdx < (int)fieldIdxList.size(); fIdx++) {
					if (*fieldIdxList.at(fIdx) == -1) {
						errStr << "ERROR: Invalid Antenna Model List file "
							  "\""
						       << filename << "\" label line missing \""
						       << fieldLabelList.at(fIdx) << "\""
						       << std::endl;
						throw std::runtime_error(errStr.str());
					}
				}

				break;
			case dataLineType:
				/**************************************************************************/
				/* prefix */
				/**************************************************************************/
				strval = fieldList.at(prefixFieldIdx);
				if (strval.empty()) {
					errStr << "ERROR: Antenna Prefix file \"" << filename
					       << "\" line " << linenum << " missing prefix"
					       << std::endl;
					throw std::runtime_error(errStr.str());
				}

				prefix = strval;
				/**************************************************************************/

				/**************************************************************************/
				/* type */
				/**************************************************************************/
				strval = fieldList.at(typeFieldIdx);
				if (strval.empty()) {
					errStr << "ERROR: Antenna Prefix file \"" << filename
					       << "\" line " << linenum << " missing type"
					       << std::endl;
					throw std::runtime_error(errStr.str());
				}

				if ((strval == "Ant") || (strval == "Antenna")) {
					type = AntennaModel::AntennaType;
				} else if ((strval == "Ref") || (strval == "Reflector")) {
					type = AntennaModel::ReflectorType;
				} else {
					errStr << "ERROR: Antenna Prefix file \"" << filename
					       << "\" line " << linenum
					       << " invalid type: " << strval << std::endl;
					throw std::runtime_error(errStr.str());
				}
				/**************************************************************************/

				/**************************************************************************/
				/* category */
				/**************************************************************************/
				strval = fieldList.at(categoryFieldIdx);
				if (strval.empty()) {
					errStr << "ERROR: Antenna Prefix file \"" << filename
					       << "\" line " << linenum << " missing category"
					       << std::endl;
					throw std::runtime_error(errStr.str());
				}

				if (strval == "HP") {
					category = AntennaModel::HPCategory;
				} else if (strval == "B1") {
					category = AntennaModel::B1Category;
				} else if ((strval == "OTHER") || (strval == "Other")) {
					category = AntennaModel::OtherCategory;
				} else {
					errStr << "ERROR: Antenna Prefix file \"" << filename
					       << "\" line " << linenum
					       << " invalid category: " << strval << std::endl;
					throw std::runtime_error(errStr.str());
				}
				/**************************************************************************/

				antennaPrefix = new AntennaPrefixClass(prefix);
				antennaPrefix->setCategory(category);
				antennaPrefix->setType(type);

				antennaPrefixList.push_back(antennaPrefix);

				break;
			case ignoreLineType:
			case unknownLineType:
				// do nothing
				break;
			default:
				CORE_DUMP;
				break;
		}
	}

	if (fp) {
		fclose(fp);
	}

	return;
}
/******************************************************************************************/

/******************************************************************************************/
/**** FUNCTION: AntennaModelMapClass::readModelMap()                                   ****/
/******************************************************************************************/
void AntennaModelMapClass::readModelMap(const std::string filename)
{
	int linenum, fIdx;
	std::string line, strval;
	FILE *fp = (FILE *)NULL;
	std::string str;
	std::string reasonIgnored;
	std::ostringstream errStr;

	int regexFieldIdx = -1;
	int modelNameFieldIdx = -1;

	std::vector<int *> fieldIdxList;
	std::vector<std::string> fieldLabelList;
	fieldIdxList.push_back(&regexFieldIdx);
	fieldLabelList.push_back("regex");
	fieldIdxList.push_back(&modelNameFieldIdx);
	fieldLabelList.push_back("Ant Model");

	int i;
	int antIdx;
	std::string name;
	std::string regexStr;
	std::regex *regExpr;

	int fieldIdx;

	if (filename.empty()) {
		throw std::runtime_error("ERROR: No Antenna Model List File specified");
	}

	if (!(fp = fopen(filename.c_str(), "rb"))) {
		str = std::string("ERROR: Unable to open Antenna Model List File \"") + filename +
		      std::string("\"\n");
		throw std::runtime_error(str);
	}

	enum LineTypeEnum { labelLineType, dataLineType, ignoreLineType, unknownLineType };

	LineTypeEnum lineType;

	linenum = 0;
	bool foundLabelLine = false;
	while (fgetline(fp, line, false)) {
		linenum++;
		std::vector<std::string> fieldList = splitCSV(line);

		lineType = unknownLineType;
		/**************************************************************************/
		/**** Determine line type                                              ****/
		/**************************************************************************/
		if (fieldList.size() == 0) {
			lineType = ignoreLineType;
		} else {
			fIdx = fieldList[0].find_first_not_of(' ');
			if (fIdx == (int)std::string::npos) {
				if (fieldList.size() == 1) {
					lineType = ignoreLineType;
				}
			} else {
				if (fieldList[0].at(fIdx) == '#') {
					lineType = ignoreLineType;
				}
			}
		}

		if ((lineType == unknownLineType) && (!foundLabelLine)) {
			lineType = labelLineType;
			foundLabelLine = 1;
		}
		if ((lineType == unknownLineType) && (foundLabelLine)) {
			lineType = dataLineType;
		}
		/**************************************************************************/

		/**************************************************************************/
		/**** Process Line                                                     ****/
		/**************************************************************************/
		bool found;
		std::string field;
		switch (lineType) {
			case labelLineType:
				for (fieldIdx = 0; fieldIdx < (int)fieldList.size(); fieldIdx++) {
					field = fieldList.at(fieldIdx);

					// std::cout << "FIELD: \"" << field << "\"" << std::endl;

					found = false;
					for (fIdx = 0;
					     (fIdx < (int)fieldLabelList.size()) && (!found);
					     fIdx++) {
						if (field == fieldLabelList.at(fIdx)) {
							*fieldIdxList.at(fIdx) = fieldIdx;
							found = true;
						}
					}
				}

				for (fIdx = 0; fIdx < (int)fieldIdxList.size(); fIdx++) {
					if (*fieldIdxList.at(fIdx) == -1) {
						errStr << "ERROR: Invalid Antenna Model Map file \""
						       << filename << "\" label line missing \""
						       << fieldLabelList.at(fIdx) << "\""
						       << std::endl;
						throw std::runtime_error(errStr.str());
					}
				}

				break;
			case dataLineType:
				/**************************************************************************/
				/* regex */
				/**************************************************************************/
				strval = fieldList.at(regexFieldIdx);
				if (strval.empty()) {
					errStr << "ERROR: Antenna Model Map file \"" << filename
					       << "\" line " << linenum << " missing regex"
					       << std::endl;
					throw std::runtime_error(errStr.str());
				}

				regexStr = strval;
				/**************************************************************************/

				/**************************************************************************/
				/* modelName */
				/**************************************************************************/
				strval = fieldList.at(modelNameFieldIdx);
				if (strval.empty()) {
					errStr << "ERROR: Antenna Model Map file \"" << filename
					       << "\" line " << linenum << " missing model name"
					       << std::endl;
					throw std::runtime_error(errStr.str());
				}

				name = strval;
				/**************************************************************************/

				regExpr = new std::regex(regexStr, std::regex_constants::icase);

				antIdx = -1;
				found = false;
				for (i = 0; (i < (int)antennaModelList.size()) && (!found); ++i) {
					if (antennaModelList[i]->name == name) {
						found = true;
						antIdx = i;
					}
				}
				if (!found) {
					errStr << "ERROR: Antenna Model Map file \"" << filename
					       << "\" line " << linenum
					       << " invalid model name: " << name << std::endl;
					throw std::runtime_error(errStr.str());
				}

				regexList.push_back(regExpr);
				antIdxList.push_back(antIdx);

				break;
			case ignoreLineType:
			case unknownLineType:
				// do nothing
				break;
			default:
				CORE_DUMP;
				break;
		}
	}

	if (fp) {
		fclose(fp);
	}

	return;
}
/******************************************************************************************/

inline bool isInvalidModelNameChar(char c)
{
	// Valid characters are 'A' - 'Z' and '0' - '9'
	bool isLetter = (c >= 'A') && (c <= 'Z');
	bool isNum = (c >= '0') && (c <= '9');
	bool valid = isLetter || isNum;
	return (!valid);
}

/******************************************************************************************/
/**** FUNCTION: AntennaModelMapClass::find()                                           ****/
/******************************************************************************************/
AntennaModelClass *AntennaModelMapClass::find(std::string antPfx,
					      std::string modelName,
					      AntennaModel::CategoryEnum &category,
					      AntennaModel::CategoryEnum modelNameBlankCategory)
{
	bool found = false;
	int antIdx;
	int i;

	category = AntennaModel::UnknownCategory;

	for (i = 0; (i < (int)regexList.size()) && (!found); ++i) {
		if (regex_match(modelName, *regexList[i])) {
			found = true;
			antIdx = antIdxList[i];
		}
	}

	AntennaModelClass *antennaModel = (AntennaModelClass *)NULL;
	if (found) {
		antennaModel = antennaModelList[antIdx];
	} else {
		/**********************************************************************************/
		/* Convert ModelName to uppercase                                                 */
		/**********************************************************************************/
		std::transform(modelName.begin(), modelName.end(), modelName.begin(), ::toupper);
		/**********************************************************************************/

		/**********************************************************************************/
		/* Remove non-alhpanumeric characters                                             */
		/**********************************************************************************/
		modelName.erase(std::remove_if(modelName.begin(),
					       modelName.end(),
					       isInvalidModelNameChar),
				modelName.end());
		/**********************************************************************************/

		/**********************************************************************************/
		/* Prepend model name prefix                                                      */
		/**********************************************************************************/
		modelName = antPfx + modelName;
		/**********************************************************************************/

		/**********************************************************************************/
		/* Match if antennaModelList contains a model that is:                            */
		/* If forceModelNameEqualFlag is false                                            */
		/*    * B1 or HP model and is a prefix of modelName                               */
		/*    * not B1 or HP and model equals modelName                                   */
		/* If forceModelNameEqualFlag is true                                             */
		/*    * model equals modelName                                                    */
		/**********************************************************************************/
		bool forceModelNameEqualFlag = true;
		for (i = 0; (i < (int)antennaModelList.size()) && (!found); ++i) {
			AntennaModelClass *m = antennaModelList[i];
			bool prefixFlag;
			if ((!forceModelNameEqualFlag) && (m->type == AntennaModel::AntennaType) &&
			    ((m->category == AntennaModel::HPCategory) ||
			     (m->category == AntennaModel::B1Category))) {
				prefixFlag = true;
			} else {
				prefixFlag = false;
			}

			if (((prefixFlag) &&
			     (modelName.compare(0, m->name.size(), m->name) == 0)) ||
			    (m->name == modelName)) {
				found = true;
				antennaModel = m;
			}
		}
		/**********************************************************************************/

		/**********************************************************************************/
		/* If still not found, determine category by checking antennaPrefixList           */
		/**********************************************************************************/
		for (i = 0; (i < (int)antennaPrefixList.size()) && (!found); ++i) {
			AntennaPrefixClass *pfx = antennaPrefixList[i];
			if ((modelName.compare(0, pfx->prefix.size(), pfx->prefix) == 0)) {
				category = pfx->category;
				found = true;
			}
		}
		/**********************************************************************************/

		/**********************************************************************************/
		/* If still not found, set category to B1 if modelName is blank                   */
		/**********************************************************************************/
		if (!found) {
			if (modelName == antPfx) {
				category = modelNameBlankCategory;
			}
		}
		/**********************************************************************************/
	}

	return (antennaModel);
}
/******************************************************************************************/
