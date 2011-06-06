// -*- C++ -*-
//
// This file is part of YODA -- Yet more Objects for Data Analysis
// Copyright (C) 2008-2011 The YODA collaboration (see AUTHORS for details)
//
#include "YODA/ReaderAIDA.h"
#include "YODA/Utils/StringUtils.h"
#include "YODA/Exceptions.h"
#include "TinyXML/tinyxml.h"

#include <iostream>
using namespace std;

namespace YODA {


  void ReaderAIDA::readDoc(std::istream& stream, vector<AnalysisObject*>& aos) {
    TiXmlDocument doc;
    stream >> doc;
    if (doc.Error()) {
      string err = "Error in " + string(doc.Value());
      err += ": " + string(doc.ErrorDesc());
      cerr << err << endl;
      throw ReadError(err);
    }

    // Return value, to be populated
    vector<AnalysisObject*> rtn;
    try {
      // Walk down tree to get to the <paper> element
      const TiXmlNode* aidaN = doc.FirstChild("aida");
      if (!aidaN) throw ReadError("Couldn't get <aida> root element");
      for (const TiXmlNode* dpsN = aidaN->FirstChild("dataPointSet"); dpsN; dpsN = dpsN->NextSibling()) {
        const TiXmlElement* dpsE = dpsN->ToElement();
        const string plotname = dpsE->Attribute("name");
        const string plotpath = dpsE->Attribute("path");

        Scatter2D* dps = new Scatter2D();
        for (const TiXmlNode* dpN = dpsN->FirstChild("dataPoint"); dpN; dpN = dpN->NextSibling()) {
          const TiXmlNode* xMeasN = dpN->FirstChild("measurement");
          const TiXmlNode* yMeasN = xMeasN->NextSibling();
          if (xMeasN && yMeasN)  {
            const TiXmlElement* xMeasE = xMeasN->ToElement();
            const TiXmlElement* yMeasE = yMeasN->ToElement();
            const string xcentreStr   = xMeasE->Attribute("value");
            const string xerrplusStr  = xMeasE->Attribute("errorPlus");
            const string xerrminusStr = xMeasE->Attribute("errorMinus");
            const string ycentreStr   = yMeasE->Attribute("value");
            const string yerrplusStr  = yMeasE->Attribute("errorPlus");
            const string yerrminusStr = yMeasE->Attribute("errorMinus");
            //if (!centreStr) throw ReadError("Couldn't get a valid bin centre");
            //if (!errplusStr) throw ReadError("Couldn't get a valid bin err+");
            //if (!errminusStr) throw ReadError("Couldn't get a valid bin err-");
            istringstream xssC(xcentreStr);
            istringstream xssP(xerrplusStr);
            istringstream xssM(xerrminusStr);
            istringstream yssC(ycentreStr);
            istringstream yssP(yerrplusStr);
            istringstream yssM(yerrminusStr);
            double xcentre, xerrplus, xerrminus, ycentre, yerrplus, yerrminus;
            xssC >> xcentre; xssP >> xerrplus; xssM >> xerrminus;
            yssC >> ycentre; yssP >> yerrplus; yssM >> yerrminus;
            dps->addPoint(xcentre, xerrminus, xerrplus, ycentre, yerrminus, yerrplus);
          } else {
            cerr << "Couldn't get <measurement> tag" << endl;
            /// @todo Throw an exception here?
          }
          aos.push_back(dps);

        }
      }
    } catch (std::exception& e) {
      cerr << e.what() << endl;
      throw;
    }
  }



  // void ReaderAIDA::readGenericAO(std::ostream& os, const Histo1D& h) {
  // }


  // void ReaderAIDA::readProfile1D(std::ostream& os, const Profile1D& p) {
  // }


  // void ReaderAIDA::readScatter2D(std::ostream& os, const Scatter2D& s) {
  // }


}
