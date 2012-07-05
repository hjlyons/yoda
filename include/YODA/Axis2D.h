// -*- C++ -*-
//
// This file is part of YODA -- Yet more Objects for Data Analysis
// Copyright (C) 2008-2012 The YODA collaboration (see AUTHORS for details)
//
#ifndef YODA_Axis2D_h
#define YODA_Axis2D_h

#include "YODA/AnalysisObject.h"
#include "YODA/Exceptions.h"
#include "YODA/Bin.h"
#include "YODA/HistoBin2D.h"
#include "YODA/Utils/cachedvector.h"
#include "YODA/Utils/MathUtils.h"
#include "YODA/Dbn2D.h"

#include <algorithm>
#include <limits>
#include <vector>

namespace YODA {


  /// @brief 1D bin container
  ///
  /// This class handles most of the low-level operations on an axis of bins
  /// arranged in a 2D grid (including gaps).
  template <typename BIN2D, typename DBN>
  class Axis2D {
  public:

    /// @name Typedefs
    //@{

    /// Bin type
    typedef BIN2D Bin;

    /// The internal bin container type. Not used for searching.
    typedef typename std::vector<Bin> Bins;

    /// The internal container type for outflow distributions.
    typedef std::map<size_t, DBN> Outflows;

    /// @brief Type used to implement a search table of low bin edges (in 2D) mapped to bin indices.
    /// An index of -1 indicates a gap interval, without a corresponding bin.
    typedef std::map<double, long int> SubBinHash;
    typedef std::map<double, SubBinHash> BinHash;

    //@}


    /// @name Constructors:
    //@{

    /// Empty constructor
    Axis2D()
    { }


    /// A constructor with specified x and y axis bin limits.
    Axis2D(const std::vector<double>& xedges, const std::vector<double>& yedges) {
      _addBins(xedges, yedges);
    }


    /// Most standard constructor accepting X/Y ranges and number of bins
    /// on each of the axis. Both axes are divided linearly.
    Axis2D(size_t nbinsX, const std::pair<double,double>& rangeX,
           size_t nbinsY, const std::pair<double,double>& rangeY) {
      _addBins(linspace(rangeX.first, rangeX.second, nbinsX),
               linspace(rangeY.first, rangeY.second, nbinsY));
    }


    /// Constructor accepting a list of bins
    Axis2D(const Bins& bins) {
      _addBins(bins);
    }


    /// State-setting constructor for persistency
    Axis2D(const Bins& bins,
           const DBN& totalDbn,
           const Outflows& outflows)
      : _bins(bins), _dbn(totalDbn), _outflows(outflows)
    {
      if (_outflows.size() != 8) {
        throw Exception("Axis2D outflow containers must have exactly 8 elements");
      }
      _updateAxis();
    }


    //@}


    /// @name Bin insertion operators
    //@{

    /// @brief Bin addition operator
    ///
    /// This operator is supplied with the extremal coordinates of just a new
    /// bin, which is then constructed and added as usual.
    void addBin(double lowX, double lowY, double highX, double highY) {
      /// @todo TODO

      /// @todo Check for overlaps
    }

    //@}


    /// @name Modifiers
    //@{

    // /// Merge a range of bins, given the bin IDs of points at the lower-left and upper-right.
    // /// @todo TODO
    // void mergeBins(size_t from, size_t to) {
    //   /// Acquire the starting/ending bins
    //   BIN2D& start = bin(from);
    //   BIN2D& end = bin(to);

    //   /// Set the bin to be added as a starting bin
    //   /// and then remove the unneeded starting bin from
    //   /// the list of bins.
    //   BIN2D temp = start;
    //   eraseBin(from);

    //   // Sanity-check of input indices
    //   if (start.midpoint().first > end.midpoint().first) {
    //     throw RangeError("The start bin has a greater x value than the end bin.");
    //   }
    //   if (start.midpoint().second > end.midpoint().second) {
    //     std::cout << "Start: " << start.midpoint().second;
    //     std::cout << " end: " << end.midpoint().second << std::endl;
    //     throw RangeError("The start bin has a greater y value than the end bin.");
    //   }

    //   /// Create a vector that will contain indexes of bins that
    //   /// will be removed after merging them with our 'main' bin.
    //   std::vector<size_t> toRemove;

    //   /// Look for lower/upper limit of the merge function operation.
    //   /// i.e.: search for index of lowEdgeY of starting bin in _binHashSparse
    //   /// and lowEdgeY of ending bin. This way we don't have to scroll through all
    //   /// the bins to check which ones we have to add.
    //   for (size_t y = (*_binHashSparse.first._cache.lower_bound(start.yMin())).second;
    //        y <= (*_binHashSparse.first._cache.lower_bound(end.yMin())).second; ++y) {
    //     /// For all the bins that are in the bounds specified in previous for
    //     for (size_t x = 0; x < _binHashSparse.first[y].second.size(); ++x) {
    //       /// If the bin lies in a rectangle produced by starting and ending bins
    //       /// (ie. the one spanned by lower-left point of starting bin and top-right
    //       /// point of ending bin) and was not merged already:
    //       if (((_binHashSparse.first[y].second[x].second.first > start.xMin() ||
    //            fuzzyEquals(_binHashSparse.first[y].second[x].second.first, start.xMin())) &&
    //           (_binHashSparse.first[y].second[x].second.second < end.xMax() ||
    //            fuzzyEquals(_binHashSparse.first[y].second[x].second.second, end.xMax())))&&
    //            !(std::find(toRemove.begin(), toRemove.end(), _binHashSparse.first[y].second[x].first) != toRemove.end()))
    //         {
    //           /// Merge it with the temp bin and mark it as ready for removal
    //           temp += bin(_binHashSparse.first[y].second[x].first);
    //           toRemove.push_back(_binHashSparse.first[y].second[x].first);
    //         }
    //     }
    //   }

    //   /// Now, drop the bins to be dropped
    //   /// Keeping in mind that the bins must be removed from the highest index
    //   /// down, otherwise we will end up removing other bins that we intend to
    //   std::sort(toRemove.begin(), toRemove.end());
    //   std::reverse(toRemove.begin(), toRemove.end());
    //   foreach(size_t remove, toRemove) eraseBin(remove);

    //   /// Add edges of our merged bin to _binHashSparse and don't create a default
    //   /// empty bin.
    //   _addEdge(temp.edges(), _binHashSparse, false);

    //   /// Add the actual merged bin to the Axis.
    //   _bins.push_back(temp);



    //   /// And regenerate the caches on _binHashSparse
    //   _binHashSparse.first.regenCache();
    //   _binHashSparse.second.regenCache();
    // }

    // /// Merge a range of bins giving start and end coordinates
    // void mergeBins(double startX, double startY, double endX, double endY) {
    //   mergeBins(binByCoord(startX, startY), binByCoord(endX, endY));
    // }

    // /// Rebin by an interger factor
    // /// NOT YET WORKING!!11!!1111
    // void rebin(size_t factorX, size_t factorY) {
    //   if (!isGrid()) throw GridError("Rebinning by a factor can only act on full grids!");
    //   if(factorX < 1 || factorY < 1) throw RangeError("Factors cannot be smaller than unity!");

    //   size_t binsInColumn = _binHashSparse.first.size() - 1;

    //   std::cout << std::endl << "Bins in column: " << binsInColumn << std::endl;
    //   std::cout <<  "Number of bins: " << _bins.size() << std::endl;
    //   size_t startIndex = 0;
    //   while(true) {
    //     size_t endIndex = startIndex;
    //     for(size_t i = 1; i < factorY; ++i){
    //       if(_hasAbove(endIndex) == 1) endIndex++;
    //       else break;
    //     }
    //     binsInColumn -= endIndex - startIndex;
    //     for(size_t i = 1; i < factorX; ++i){
    //       if(endIndex + binsInColumn < _bins.size()) endIndex += binsInColumn;
    //       else break;
    //     }
    //     if(endIndex + 1 >= _bins.size()) break;
    //     mergeBins(startIndex, endIndex);
    //     if(startIndex + 1 < _bins.size()) startIndex++;
    //     else break;

    //     if(_hasAbove(startIndex-1) == 0) binsInColumn = _binHashSparse.first.size() -1;
    //   }

    // }


    /// Reset the axis statistics
    void reset() {
      _dbn.reset();
      foreach(Bin bin, _bins) {
        bin.reset();
      }
    }

    //@}


    /// @name Accessors
    //@{

    /// Get the total number of bins
    /// @todo Can't this just be numBins?
    const size_t numBins() const {
      return _bins.size();
    }

    /// Get the number of bins along X axis
    const size_t numBinsX() const {
      return (numBins() > 0) ? _binhash.size() : 0;
    }

    /// Get the number of bins along Y axis
    const size_t numBinsY() const {
      return (numBins() > 0) ? _binhash.begin()->second.size() : 0;
    }


    /// Get the value of the lowest x-edge on the axis
    const double lowEdgeX() const {
      if (numBins() == 0) throw RangeError("This axis contains no bins and so has no defined range");
      return _binhash.begin()->first;
    }
    /// A alias for lowEdgeX()
    double xMin() const { return lowEdgeX();}

    /// Get the value of the highest x-edge on the axis
    const double highEdgeX() const {
      if (numBins() == 0) throw RangeError("This axis contains no bins and so has no defined range");
      return (--_binhash.end())->first;
    }
    /// Alias for highEdgeX()
    double xMax() const { return highEdgeX();}

    /// Get the value of the lowest y-edge on the axis
    const double lowEdgeY() const {
      if (numBins() == 0) throw RangeError("This axis contains no bins and so has no defined range");
      return _binhash.begin()->second.begin()->first;
    }
    /// A alias for lowEdgeY()
    double yMin() const { return lowEdgeY();}

    /// Get the value of the highest y-edge on the axis
    const double highEdgeY() const {
      if (numBins() == 0) throw RangeError("This axis contains no bins and so has no defined range");
      return (--_binhash.begin()->second.end())->first;
    }
    /// Alias for highEdgeY()
    double yMax() const { return highEdgeY();}


    /// Get the bins (non-const version)
    Bins& bins() {
      return _bins;
    }

    /// Get the bins (const version)
    const Bins& bins() const {
      return _bins;
    }


    /// @brief Get an outflow (non-const version)
    ///
    /// Two indices are used, for x and y: -1 = underflow, 0 = in-range, and +1 = overflow.
    /// (0,0) is not a valid overflow index pair, since it is in range for both x and y.
    DBN& outflow(size_t ix, size_t iy) {
      if (ix == 0 && iy == 0) throw UserError("(0,0) is not a valid Axis2D overflow index");
      if (abs(ix) > 1 || abs(iy) > 1) throw UserError("Axis2D overflow indices are -1, 0, 1");
      size_t realindex = 3*(ix+1) + (iy+1);
      return _outflows[realindex];
    }

    /// @brief Get an outflow (const version)
    ///
    /// Two indices are used, for x and y: -1 = underflow, 0 = in-range, and +1 = overflow.
    /// (0,0) is not a valid overflow index pair, since it is in range for both x and y.
    const DBN& outflow(size_t ix, size_t iy) const {
      if (ix == 0 && iy == 0) throw UserError("(0,0) is not a valid Axis2D overflow index");
      if (abs(ix) > 1 || abs(iy) > 1) throw UserError("Axis2D overflow indices are -1, 0, 1");
      size_t realindex = 3*(ix+1) + (iy+1);
      return _outflows.find(realindex)->second;
    }


    /// Get the bin with a given index (non-const version)
    BIN2D& bin(size_t index) {
      if (index >= _bins.size()) throw RangeError("Bin index out of range.");
      return _bins[index];
    }

    /// Get the bin with a given index (const version)
    const BIN2D& bin(size_t index) const {
      if (index >= _bins.size()) throw RangeError("Bin index out of range.");
      return _bins[index];
    }


    /// Get a bin at given coordinates (non-const version)
    BIN2D& binByCoord(double x, double y) {
      const int ret = getBinIndex(x, y);
      if (ret == -1) throw RangeError("No bin found!!");
      return bin(ret);
    }

    /// Get a bin at given coordinates (const version)
    const BIN2D& binByCoord(double x, double y) const {
      const int ret = getBinIndex(x, y);
      if (ret == -1) throw RangeError("No bin found!!");
      return bin(ret);
    }


    /// Get a bin at given coordinates (non-const version)
    BIN2D& binByCoord(std::pair<double, double>& coords) {
      return binByCoord(coords.first, coords.second);
    }

    /// Get a bin at given coordinates (const version)
    BIN2D& binByCoord(std::pair<double, double>& coords) const {
      return binByCoord(coords.first, coords.second);
    }


    /// Get the total distribution (non-const version)
    DBN& totalDbn() {
      return _dbn;
    }

    /// Get the total distribution (const version)
    const DBN& totalDbn() const {
      return _dbn;
    }


    /// @brief Bin index finder
    ///
    /// Looks through all the bins to see which one contains the point of
    /// interest.
    const long getBinIndex(double coordX, double coordY) const {
      // First check that we are within the axis bounds at all
      if (coordX < lowEdgeX() || coordX > highEdgeX()) return -1;
      if (coordY < lowEdgeY() || coordY > highEdgeY()) return -1;
      // Then return the lower-edge lookup (in both directions) from the hash map.
      // NB. both upper_bound and lower_bound return values *greater* than (or equal) to coord,
      // so we have to step back one iteration to get to the lower-or-equal containing bin edge.
      BinHash::const_iterator itaboveX = _binhash.upper_bound(coordX);
      SubBinHash::const_iterator itaboveY = (--itaboveX)->second.upper_bound(coordY);
      int index = (--itaboveY)->second;
      return index;
    }


    // /// Fast column number searcher
    // const size_t getBinColumn(size_t index) const {
    //   // Check if assumptions are reasonable
    //   if (!_isGrid) throw GridError("This operation can only be performed when an array is a grid!");
    //   if (index >= _bins.size()) throw RangeError("Index is bigger than the size of bins vector!");

    //   // Find the column number
    //   size_t ret  = (*_binHashSparse.first._cache.lower_bound(approx(bin(index).yMin()))).second;
    //   return ret;
    // }


    // /// Fast row number searcher
    // const size_t getBinRow(size_t index) const {
    //   // Check if assumptions are reasonable
    //   if (!_isGrid) throw GridError("This operation can only be performed when an array is a grid!");
    //   if (index >= _bins.size()) throw RangeError("Index is bigger than the size of bins vector!");

    //   // Find the row number
    //   size_t ret  = (*_binHashSparse.second._cache.lower_bound(approx(bin(index).xMin()))).second;
    //   return ret;
    // }


    /// @brief Bin eraser
    ///
    /// Removes a bin at a position.
    void eraseBin(size_t index) {
      // Check the correctness of assumptions
      if (index >= numBins()) throw RangeError("Index is bigger than the size of bins vector!");

      /// @todo Check for overlaps
      /// @todo If within the current bounds, create and rehash
      /// @todo If outside the bounds, check for grid compatibility, create and rehash
    }
    //@}


    /// @name Scaling operations
    //@{

    /// @brief Rescale the axes
    ///
    /// Scales the axis with a given scale.
    /// @todo Add a specific check for a scaling of 1.0, to avoid doing work when no scaling is wanted.
    void scaleXY(double scaleX, double scaleY) {
      foreach(Bin bin, _bins) {
        bin.scaleXY(scaleX, scaleY);
      }
      _dbn.scaleXY(scaleX, scaleY);
      // Outflows
      for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
          _outflows[3*i+j].scaleXY(scaleX, scaleY);;
        }
      }
      // Rehash
      _updateAxis();
    }


    /// Scales the bin weights
    void scaleW(double scalefactor) {
      foreach(Bin bin, _bins) {
        bin.scaleW(scalefactor);
      }
      _dbn.scaleW(scalefactor);
      // Outflows
      for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
          _outflows[3*i+j].scaleW(scalefactor);
        }
      }
    }

    //@}


    /// @name Operators
    //@{

    /// Equality operator (on binning only)
    bool operator == (const Axis2D& other) const {
      /// @todo TODO
      return true;
    }


    /// Non-equality operator
    bool operator != (const Axis2D& other) const {
      return ! operator == (other);
    }


    /// @brief Addition operator
    /// At this stage it is only possible to add histograms with the same binnings.
    /// @todo Compatible but not equal binning to come soon.
    Axis2D<BIN2D, DBN>& operator += (const Axis2D<BIN2D, DBN>& toAdd) {
      if (*this != toAdd) {
        throw LogicError("YODA::Axis2D: Cannot add axes with different binnings.");
      }
      for (size_t i = 0; i < bins().size(); ++i) {
        bin(i) += toAdd.bin(i);
      }
      _dbn += toAdd._dbn;
      return *this;
    }


    /// Subtraction operator
    ///
    /// At this stage it is only possible to subtract histograms with the same binnings.
    /// @todo Compatible but not equal binning to come soon.
    Axis2D<BIN2D, DBN>& operator -= (const Axis2D<BIN2D, DBN>& toSubtract) {
      if (*this != toSubtract) {
        throw LogicError("YODA::Axis2D: Cannot add axes with different binnings.");
      }
      for (size_t i = 0; i < bins().size(); ++i) {
        bin(i) -= toSubtract.bin(i);
      }
      _dbn -= toSubtract._dbn;
      return *this;
    }

    //@}


  private:

    /// Add new bins, constructed from two sorted vectors of edges, to the axis
    void _addBins(const std::vector<double>& xbinedges, const std::vector<double>& ybinedges) {
      /// @todo Check that vectors are sorted?

      /// @todo Check whether there is overlap with any existing edges in either direction
      // if (_edgeInRange(binedges.front(), binedges.back())) {
      //   throw RangeError("New bin range overlaps with existing bin edges");
      // }

      // Create and add bins
      for (size_t i = 0; i < xbinedges.size() - 1; ++i) {
        for (size_t j = 0; j < ybinedges.size() - 1; ++j) {
          _bins.push_back( BIN2D(std::make_pair(xbinedges[i], xbinedges[i+1]),
                                 std::make_pair(ybinedges[j], ybinedges[j+1])) );
        }
      }
      _updateAxis();
    }


    /// Add new bins to the axis
    void _addBins(const Bins& bins) {
      for (size_t i = 0; i < bins.size(); ++i) {

        /// @todo Check for 2D edge overlaps

        // if (_edgeInRange(bins[i].xMin(), bins[i].xMax())) {
        //   throw RangeError("New bin range overlaps with existing bin edges");
        // }
        _bins.push_back(bins[i]);
      }
      _updateAxis();
    }


    /// Sort the bins vector, and regenerate the bin hash
    ///
    /// The bin hash is purely for searching, and is generated from the bins list only.
    void _updateAxis() {
      /// First, set up the collection of low edges based on all unique edges
      std::vector<double> xedges, yedges;
      std::sort(_bins.begin(), _bins.end());
      for (size_t i = 0; i < numBins(); ++i) {
        xedges.push_back(bin(i).xMin());
        xedges.push_back(bin(i).xMax()); // only the unique max edges will "survive"
        yedges.push_back(bin(i).yMin());
        yedges.push_back(bin(i).xMax()); // only the unique max edges will "survive"
      }
      std::sort(xedges.begin(), xedges.end());
      std::vector<double>::iterator itx = std::unique(xedges.begin(), xedges.end());
      xedges.resize(itx - xedges.begin());
      std::sort(yedges.begin(), yedges.end());
      std::vector<double>::iterator ity = std::unique(yedges.begin(), yedges.end());
      yedges.resize(ity - yedges.begin());

      // Create a double-map hash based on the two sets of low edges. Initialize with null bin indices.
      _binhash.clear();
      for (size_t ix = 0; ix < xedges.size() - 1; ++ix) {
        _binhash[xedges[ix]] = SubBinHash();
        for (size_t iy = 0; iy < yedges.size() - 1; ++iy) {
          _binhash[xedges[ix]][yedges[iy]] = -1;
        }
      }

      // Loop over bins, setting each one's non-null bin index appropriately in the double-hashmap.
      for (size_t ib = 0; ib < numBins(); ++ib) {

        // Find the axis low edges contained within this bin
        const double xmin(bin(ib).xMin()), ymin(bin(ib).xMin());
        std::vector<double> xlowedges_in_bin, ylowedges_in_bin;
        /// @todo Use std::upper/lower_bound?
        for (size_t ix = 0; ix < xedges.size() - 1; ++ix) {
          if (xedges[ix] >= xmin) xlowedges_in_bin.push_back(xedges[ix]);
        }
        /// @todo Use std::upper/lower_bound?
        for (size_t iy = 0; iy < yedges.size() - 1; ++iy) {
          if (yedges[iy] >= ymin) ylowedges_in_bin.push_back(yedges[iy]);
        }

        // Set bin indices
        for (std::vector<double>::const_iterator x = xlowedges_in_bin.begin(); x != xlowedges_in_bin.end(); ++x) {
          for (std::vector<double>::const_iterator y = ylowedges_in_bin.begin(); y != ylowedges_in_bin.end(); ++y) {
            _binhash[*x][*y] = ib;
          }
        }

      }

      // // DEBUG
      // for (size_t ix = 0; ix < xedges.size() - 1; ++ix) {
      //   for (size_t iy = 0; iy < yedges.size() - 1; ++iy) {
      //     std::cout << xedges[ix] << "  " << yedges[iy] << "   " << _binhash[xedges[ix]][yedges[iy]] << std::endl;
      //   }
      // }

    }


    /// @todo Convert from 1D to 2D
    // /// Check if there are any bin edges between values @a from and @a to.
    // bool _edgeInRange(double from, double to) const {
    //   return (--_binhash.upper_bound(from)) != (--_binhash.lower_bound(to));
    // }


    /// @todo Convert from 1D to 2D
    // /// Check if there are any gaps in the axis' binning between bin indices @a from and @a to, inclusive.
    // bool _gapInRange(size_t ifrom, size_t ito) const {
    //   assert(ifrom < numBins() && ito < numBins() && ifrom <= ito);
    //   if (ifrom == ito) return false;
    //   BinHash::const_iterator start = (--_binhash.upper_bound(bin(ifrom).midpoint()));
    //   BinHash::const_iterator end = _binhash.upper_bound(bin(ifrom).midpoint());
    //   for (BinHash::const_iterator it = start; it != end; ++it) {
    //     if (it->second == -1) return true;
    //   }
    //   return false;
    // }


  private:

    /// @name Data structures
    //@{

    /// Bins contained in this histogram
    Bins _bins;

    /// Total distribution
    DBN _dbn;

    /// Outflow distributions, indexed clockwise from top
    Outflows _outflows;

    /// Cached bin edges for searching
    BinHash _binhash;

    //@}

  };


  /// @name Operators
  //@{

  /// Addition operator
  template <typename BIN2D, typename DBN>
  inline Axis2D<BIN2D, DBN> operator + (const Axis2D<BIN2D, DBN>& first, const Axis2D<BIN2D, DBN>& second) {
    Axis2D<BIN2D, DBN> tmp = first;
    tmp += second;
    return tmp;
  }


  /// Subtraction operator
  template <typename BIN2D, typename DBN>
  inline Axis2D<BIN2D, DBN> operator - (const Axis2D<BIN2D, DBN>& first, const Axis2D<BIN2D, DBN>& second) {
    Axis2D<BIN2D, DBN> tmp = first;
    tmp -= second;
    return tmp;
  }

  //@}


}

#endif
