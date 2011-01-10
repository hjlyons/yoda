// -*- C++ -*-
//
// This file is part of YODA -- Yet more Objects for Data Analysis
// Copyright (C) 2008-2011 The YODA collaboration (see AUTHORS for details)
//
#ifndef YODA_Axis1D_h
#define YODA_Axis1D_h

#include "YODA/AnalysisObject.h"
#include "YODA/Exceptions.h"
#include "YODA/Bin.h"
#include "YODA/Utils/sortedvector.h"
#include "YODA/Utils/MathUtils.h"
#include <string>
#include <cassert>
#include <cmath>
#include <algorithm>

using namespace std;

namespace YODA {


  template <typename BIN>
  class Axis1D {
  public:

    typedef typename Utils::sortedvector<BIN> Bins;


    /// @name Helper functions to make bin edge vectors (see @file MathUtils.h)
    //@{

    static inline std::vector<double> mkBinEdgesLin(double start, double end, size_t nbins) {
      return linspace(start, end, nbins);
    }

    static inline std::vector<double> mkBinEdgesLog(double start, double end, size_t nbins) {
      return logspace(start, end, nbins);
    }

    //@}


  private:

    /// @todo Remove
    void _mkBinHash() {
      for (size_t i = 0; i < numBins(); i++) {
        // Insert upper bound mapped to bin ID
        _binHash.insert(make_pair(_cachedBinEdges[i+1],i));
      }
    }


    void _mkAxis(const vector<double>& binedges) {
      const size_t nbins = binedges.size() - 1;
      for (size_t i = 0; i < nbins; ++i) {
        _bins.push_back( BIN(binedges.at(i), binedges.at(i+1)) );
      }

      /// @todo Remove
      _cachedBinEdges = binedges;
      std::sort(_cachedBinEdges.begin(), _cachedBinEdges.end());
      _mkBinHash();
    }


    void _mkAxis(const Bins& bins) {
      _bins = bins;

      /// @todo Remove
      for (size_t i = 0; i < bins.size(); ++i) {
        _cachedBinEdges.push_back(bins.at(i).lowEdge());
      }
      _cachedBinEdges.push_back(bins.back().highEdge());
      _mkBinHash();
    }


  public:

    /// Constructor with a list of bin edges
    /// @todo Accept a general iterable and remove this silly special-casing for std::vector
    Axis1D(const vector<double>& binedges) {
      assert(binedges.size() > 1);
      _mkAxis(binedges);
    }


    /// Constructor with histogram limits, number of bins, and a bin distribution enum
    Axis1D(size_t nbins, double lower, double upper) {
      _mkAxis(linspace(nbins, lower, upper));
    }


    /// @todo Accept a general iterable and remove this silly special-casing for std::vector
    Axis1D(const vector<BIN>& bins) {
      assert(!bins.empty());
      Bins sbins;
      for (typename vector<BIN>::const_iterator b = bins.begin(); b != bins.end(); ++b) {
        sbins.insert(*b);
      }
      _mkAxis(sbins);
    }


    /// @todo Accept a general iterable (and remove this internal detail special-casing?)
    Axis1D(const Bins& bins) {
      assert(!bins.empty());
      _mkAxis(bins);
    }


    /////////////////////


  public:

    unsigned int numBins() const {
      return _bins.size();
    }


    Bins& bins() {
      return _bins;
    }


    const Bins& bins() const {
      return _bins;
    }


    pair<double,double> binEdges(size_t binId) const {
      assert(binId < numBins());
      return make_pair(_cachedBinEdges[binId], _cachedBinEdges[binId+1]);
    }


    double lowEdge() const {
      /// @todo Check that this is still okay when bins are auto-sorted
      return _bins.front().lowEdge();
    }

    double highEdge() const {
      /// @todo Check that this is still okay when bins are auto-sorted
      return _bins.back().highEdge();
    }


    BIN& bin(size_t index) {
      if (index >= numBins())
        throw RangeError("YODA::Histo: index out of range");
      return _bins[index];
    }


    const BIN& bin(size_t index) const {
      if (index >= numBins())
        throw RangeError("YODA::Histo: index out of range");
      /// @todo Fix via "indexed set" of bins
      return _bins[index];
    }


    BIN& binByCoord(double x) {
      return bin(findBinIndex(x));
    }


    const BIN& binByCoord(double x) const {
      return bin(findBinIndex(x));
    }


    size_t findBinIndex(double coord) const {
      /// @todo Improve!
      if (coord < _cachedBinEdges[0] || coord >= _cachedBinEdges[numBins()]) {
        throw RangeError("Coordinate is outside the valid range: you should request the underlow or overflow");
      }
      size_t i = _binHash.upper_bound(coord)->second;
      return i;
    }


    void reset() {
      _underflow.reset();
      _overflow.reset();
      for (typename Bins::iterator b = _bins.begin(); b != _bins.end(); ++b) {
        b->reset();
      }
    }


    /// Scale the axis coordinates (i.e. bin edges)
    /// @todo Base this on a general transformation of the axis coordinates via a supplied function (object)
    void scale(double scalefactor) {
      /// @todo Implement!
      throw std::runtime_error("Axis coordinate transformations not yet implemented! Pester me, please.");
    }


    void scaleW(double scalefactor) {
      _underflow.scaleW(scalefactor);
      _overflow.scaleW(scalefactor);
      for (typename Bins::iterator b = _bins.begin(); b != _bins.end(); ++b) {
        b->scaleW(scalefactor);
      }
    }


  public:

    bool operator == (const Axis1D& other) const {
      /// @todo Need/want to compare bin hash?
      return
        _cachedBinEdges == other._cachedBinEdges &&
        _binHash == other._binHash;
    }


    bool operator != (const Axis1D& other) const {
      return ! operator == (other);
    }


    Axis1D<BIN>& operator += (const Axis1D<BIN>& toAdd) {
      if (*this != toAdd) {
        throw LogicError("YODA::Histo1D: Cannot add axes with different binnings.");
      }
      for (size_t i = 0; i < bins().size(); ++i) {
        bins().at(i) += toAdd.bins().at(i);
      }
      _underflow += toAdd._underflow;
      _overflow  += toAdd._overflow;
      return *this;
    }


    Axis1D<BIN>& operator -= (const Axis1D<BIN>& toSubtract) {
      if (*this != toSubtract) {
        throw LogicError("YODA::Histo1D: Cannot subtract axes with different binnings.");
      }
      for (size_t i = 0; i < bins().size(); ++i) {
        bins().at(i) += toSubtract.bins().at(i);
      }
      _underflow -= toSubtract._underflow;
      _overflow  -= toSubtract._overflow;
      return *this;
    }


  private:


    /// @todo Store bins in a more flexible (and sorted) way
    /// @todo Check non-overlap of bins
    /// @todo Bin access by index
    /// @todo Overall y-dbn for profiles?


    /// @name Bin data
    //@{

    /// The bins contained in this histogram
    Bins _bins;

    /// A distribution counter for overflow fills
    Dbn1D _underflow;
    /// A distribution counter for underlow fills
    Dbn1D _overflow;

    /// A distribution counter for the whole histogram
    Dbn1D _dbn;

    /// Bin edges: lower edges, except last entry,
    /// which is the high edge of the last bin
    std::vector<double> _cachedBinEdges;

    /// Map for fast bin lookup
    std::map<double,size_t> _binHash;
    //@}

  };



  template <typename BIN>
  Axis1D<BIN> operator + (const Axis1D<BIN>& first, const Axis1D<BIN>& second) {
    Axis1D<BIN> tmp = first;
    tmp += second;
    return tmp;
  }


  template <typename BIN>
  Axis1D<BIN> operator - (const Axis1D<BIN>& first, const Axis1D<BIN>& second) {
    Axis1D<BIN> tmp = first;
    tmp -= second;
    return tmp;
  }



}

#endif