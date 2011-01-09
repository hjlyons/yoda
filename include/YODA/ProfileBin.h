// -*- C++ -*-
//
// This file is part of YODA -- Yet more Objects for Data Analysis
// Copyright (C) 2008-2011 The YODA collaboration (see AUTHORS for details)
//
#ifndef YODA_ProfileBin_h
#define YODA_ProfileBin_h

#include "YODA/Bin.h"
#include "YODA/Exceptions.h"

namespace YODA {


  /// A  one-dimensional profile histogram.
  class ProfileBin : public Bin {

    /// Profile1D is a friend to add/subtract bins
    friend class Profile1D;


  public:

    /// @name Constructors
    //@{

    /// Constructor giving bin low and high edges.
    ProfileBin(double lowedge, double highedge);

    /// Constructor giving bin low and high edges as a pair.
    ProfileBin(std::pair<double,double> edges);

    //@}


    /// @name Modifiers
    //@{

    /// Fill histo by value and weight.
    void fill(double x, double d, double weight=1.0);

    /// Fill histo with @a weight at bin midpoint.
    void fillBin(double d, double weight=1.0);

    /// Reset the bin.
    void reset();

    /// Rescale as if all fill weights had been different by factor @a scalefactor.
    void scaleW(double scalefactor) {
      _xdbn.scaleW(scalefactor);
      _ydbn.scaleW(scalefactor);
    }

    //@}


  public:

    /// @name Bin content info
    //@{

    /// Make the "y" explicit?

    double mean() const;

    double stdDev() const;

    double variance() const;

    double stdErr() const;

    //@}


  public:

    /// Add two bins (for use by Profile1D).
    ProfileBin& operator += (const ProfileBin&);

    /// Subtract two bins
    ProfileBin& operator -= (const ProfileBin&);


  protected:

    /// Add two bins (internal, explicitly named version)
    ProfileBin& add(const ProfileBin&);

    /// Subtract one bin from another (internal, explicitly named version)
    ProfileBin& subtract(const ProfileBin&);


  public:

    /// The sum of y*weight
    double sumWY() const;

    /// The sum of y^2 * weight
    double sumWY2() const;


  private:

    // Distribution of weighted data values
    Dbn1D _ydbn;


  };


  ProfileBin operator + (const ProfileBin& a, const ProfileBin& b);

  ProfileBin operator - (const ProfileBin& a, const ProfileBin& b);

}

#endif
