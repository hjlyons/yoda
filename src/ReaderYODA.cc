// -*- C++ -*-
//
// This file is part of YODA -- Yet more Objects for Data Analysis
// Copyright (C) 2008-2014 The YODA collaboration (see AUTHORS for details)
//
#include "YODA/ReaderYODA.h"
#include "YODA/Utils/StringUtils.h"
#include "YODA/Exceptions.h"

#include <iostream>
using namespace std;

namespace YODA {

  qi::symbols<char, int> ReaderYODA::bgroup;
  qi::symbols<char, int> ReaderYODA::egroup;
  ReaderYODA::histo1d ReaderYODA::_histo1d;
  ReaderYODA::profile1d ReaderYODA::_profile1d;
  ReaderYODA::scatter2d ReaderYODA::_scatter2d;
  map<string, string> ReaderYODA::_annotations;


  namespace {

    // Portable version of getline taken from
    // http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
    istream& safe_getline(istream& is, string& t) {
      t.clear();

      // The characters in the stream are read one-by-one using a std::streambuf.
      // That is faster than reading them one-by-one using the std::istream.
      // Code that uses streambuf this way must be guarded by a sentry object.
      // The sentry object performs various tasks,
      // such as thread synchronization and updating the stream state.
      istream::sentry se(is, true);
      streambuf* sb = is.rdbuf();

      for (;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
          return is;
        case '\r':
          if (sb->sgetc() == '\n')
            sb->sbumpc();
          return is;
        case EOF:
          // Also handle the case when the last line has no line ending
          if (t.empty())
            is.setstate(ios::eofbit);
          return is;
        default:
          t += (char)c;
        }
      }
    }

  }


  void ReaderYODA::_readDoc(std::istream& stream, vector<AnalysisObject*>& aos) {

    // These are the context groups we know. We need
    // that map to dynamically change the parser depending
    // on what we read in.
    std::map<int, std::string> groups;
    groups[1] = "YODA_HISTO1D";
    groups[2] = "YODA_HISTO2D";
    groups[3] = "YODA_PROFILE1D";
    groups[4] = "YODA_PROFILE2D";
    groups[5] = "YODA_SCATTER1D";
    groups[6] = "YODA_SCATTER2D";
    groups[7] = "YODA_SCATTER3D";

    // Initialize the group parser
    std::pair <int, std::string> pis;  // To make boost's foreach happy
    foreach(pis, groups) {
      bgroup.add(pis.second, pis.first);
    }

    // The grammars for content (yoda) and context (group)
    yoda_grammar<std::string::iterator, ascii::space_type> yoda_parser;
    group_grammar<std::string::iterator> group_parser;

    // Now loop over all lines of the input file
    int context = 0;
    bool contextchange = false;
    std::string s;
    while (safe_getline(stream, s)) {
      // First check if we found a "# BEGIN ..." or "# END ..." line.
      // This marks a context change.
      int newcontext = 0;
      // if (qi::parse(s.begin(), s.end(), group_parser, newcontext)) { //< Only supported in Boost 1.47+
      std::string::iterator it1 = s.begin();
      if (qi::parse(it1, s.end(), group_parser, newcontext)) { //< End patch
        context = newcontext;
        if (context > 0) {
          // We are inside a group now, so we are looking for the corresponding
          // END and ignore all BEGINs
          bgroup.clear();
          egroup.add(groups[context], -context);
        }
        if (context < 0) {
          // We are outside a group, so we are looking for any BEGIN and ignore
          // all ENDs
          egroup.remove(groups[-context]);
          foreach(pis, groups) {
            bgroup.add(pis.second, pis.first);
          }
          contextchange = true;
        }
      }

      // Depending on the context, we either want to parse the line as data,
      // or to write out what we parsed so far (when leaving a group).
      switch (context) {
        case 1:  // we are inside YODA_HISTO1D
        case 2:  // we are inside YODA_HISTO2D
        case 3:  // we are inside YODA_PROFILE1D
        case 4:  // we are inside YODA_PROFILE2D
        case 5:  // we are inside YODA_SCATTER1D
        case 6:  // we are inside YODA_SCATTER2D
        case 7:  // we are inside YODA_SCATTER3D
          // if (! qi::phrase_parse(s.begin(), s.end(), yoda_parser, ascii::space) ) { //< Only supported in Boost 1.47+
          { //< Why the explicit scoping? Added by supplied patch from Andrii Verbytskyi
            std::string::iterator it2 = s.begin();
            if (! qi::phrase_parse(it2, s.end(), yoda_parser, ascii::space) ) { //< End patch
              std::cerr << "failed parsing this line:\n" << s << std::endl;
            }
          } //< End patch scoping
          break;
        case -1: // we left YODA_HISTO1D
          if (contextchange) {
            YODA::Histo1D* h = new YODA::Histo1D(_histo1d.bins, _histo1d.dbn_tot, _histo1d.dbn_uflow, _histo1d.dbn_oflow);
            std::pair <std::string, std::string> pss;  // to make boost's foreach happy
            foreach (pss, _annotations) {
              h->setAnnotation(pss.first, pss.second);
            }
            aos.push_back(h);
            cleanup();
            contextchange = false;
          }
          break;
        case -3: // we left YODA_PROFILE1D
          if (contextchange) {
            YODA::Profile1D* h = new YODA::Profile1D(_profile1d.bins, _profile1d.dbn_tot, _profile1d.dbn_uflow, _profile1d.dbn_oflow);
            std::pair <std::string, std::string> pss;  // to make boost's foreach happy
            foreach (pss, _annotations) {
              h->setAnnotation(pss.first, pss.second);
            }
            aos.push_back(h);
            cleanup();
            contextchange = false;
          }
          break;
        case -6: // we left YODA_SCATTER2D
          if (contextchange) {
            YODA::Scatter2D* h = new YODA::Scatter2D(_scatter2d.points);
            std::pair <std::string, std::string> pss;  // to make boost's foreach happy
            foreach (pss, _annotations) {
              h->setAnnotation(pss.first, pss.second);
            }
            aos.push_back(h);
            cleanup();
            contextchange = false;
          }
          break;
      }
    }
  }


}
