// -*- C++ -*-
//
// ======================================================================
//
//                           Brad T. Aagaard
//                        U.S. Geological Survey
//
// {LicenseText}
//
// ======================================================================
//

// DO NOT EDIT THIS FILE
// This file was generated from python application elasticityimplicit.

#include "ElasticityImplicitData2DLinear.hh"

const int pylith::feassemble::ElasticityImplicitData2DLinear::_spaceDim = 2;

const int pylith::feassemble::ElasticityImplicitData2DLinear::_cellDim = 2;

const int pylith::feassemble::ElasticityImplicitData2DLinear::_numVertices = 3;

const int pylith::feassemble::ElasticityImplicitData2DLinear::_numCells = 1;

const int pylith::feassemble::ElasticityImplicitData2DLinear::_numBasis = 3;

const int pylith::feassemble::ElasticityImplicitData2DLinear::_numQuadPts = 1;

const char* pylith::feassemble::ElasticityImplicitData2DLinear::_matType = "ElasticPlaneStrain";

const char* pylith::feassemble::ElasticityImplicitData2DLinear::_matDBFilename = "data/elasticplanestrain.spatialdb";

const int pylith::feassemble::ElasticityImplicitData2DLinear::_matId = 0;

const char* pylith::feassemble::ElasticityImplicitData2DLinear::_matLabel = "elastic strain 2-D";

const double pylith::feassemble::ElasticityImplicitData2DLinear::_dt =   1.00000000e-02;

const double pylith::feassemble::ElasticityImplicitData2DLinear::_vertices[] = {
  2.00000000e-01, -4.00000000e-01,
  3.00000000e-01,  5.00000000e-01,
 -1.00000000e+00, -2.00000000e-01,
};

const int pylith::feassemble::ElasticityImplicitData2DLinear::_cells[] = {
0,1,2,
};

const double pylith::feassemble::ElasticityImplicitData2DLinear::_verticesRef[] = {
 -1.00000000e+00, -1.00000000e+00,
  1.00000000e+00, -1.00000000e+00,
 -1.00000000e+00,  1.00000000e+00,
};

const double pylith::feassemble::ElasticityImplicitData2DLinear::_quadPts[] = {
  3.33333333e-01,  3.33333333e-01,
};

const double pylith::feassemble::ElasticityImplicitData2DLinear::_quadWts[] = {
  5.00000000e-01,
};

const double pylith::feassemble::ElasticityImplicitData2DLinear::_basis[] = {
  3.33333333e-01,  3.33333333e-01,
  3.33333333e-01,};

const double pylith::feassemble::ElasticityImplicitData2DLinear::_basisDerivRef[] = {
 -1.00000000e+00, -1.00000000e+00,
  1.00000000e+00,  0.00000000e+00,
  0.00000000e+00,  1.00000000e+00,
};

const double pylith::feassemble::ElasticityImplicitData2DLinear::_fieldTpdt[] = {
  1.30000000e+00, -9.00000000e-01,
  1.40000000e+00,  1.50000000e+00,
  5.00000000e-01, -9.00000000e-01,
};

const double pylith::feassemble::ElasticityImplicitData2DLinear::_fieldT[] = {
  1.60000000e+00, -8.00000000e-01,
  9.00000000e-01,  7.00000000e-01,
 -2.00000000e-01, -1.10000000e+00,
};

const double pylith::feassemble::ElasticityImplicitData2DLinear::_fieldTmdt[] = {
  8.00000000e-01,  1.00000000e-01,
  5.00000000e-01,  3.00000000e-01,
 -1.00000000e-01, -6.00000000e-01,
};

const double pylith::feassemble::ElasticityImplicitData2DLinear::_valsResidual[] = {
 -4.40727273e+10,  1.63800000e+11,
 -1.81636364e+10, -1.54200000e+11,
  6.22363636e+10, -9.60000000e+09,
};

const double pylith::feassemble::ElasticityImplicitData2DLinear::_valsJacobian[] = {
  3.15681818e+10, -1.86136364e+10,
 -4.90909091e+09,  9.68181818e+09,
 -2.66590909e+10,  8.93181818e+09,
 -1.86136364e+10,  7.24772727e+10,
  2.18181818e+09, -6.28636364e+10,
  1.64318182e+10, -9.61363636e+09,
 -4.90909091e+09,  2.18181818e+09,
  1.14545455e+10,  4.90909091e+09,
 -6.54545455e+09, -7.09090909e+09,
  9.68181818e+09, -6.28636364e+10,
  4.90909091e+09,  5.91818182e+10,
 -1.45909091e+10,  3.68181818e+09,
 -2.66590909e+10,  1.64318182e+10,
 -6.54545455e+09, -1.45909091e+10,
  3.32045455e+10, -1.84090909e+09,
  8.93181818e+09, -9.61363636e+09,
 -7.09090909e+09,  3.68181818e+09,
 -1.84090909e+09,  5.93181818e+09,
};

pylith::feassemble::ElasticityImplicitData2DLinear::ElasticityImplicitData2DLinear(void)
{ // constructor
  spaceDim = _spaceDim;
  cellDim = _cellDim;
  numVertices = _numVertices;
  numCells = _numCells;
  numBasis = _numBasis;
  numQuadPts = _numQuadPts;
  matType = const_cast<char*>(_matType);
  matDBFilename = const_cast<char*>(_matDBFilename);
  matId = _matId;
  matLabel = const_cast<char*>(_matLabel);
  dt = _dt;
  vertices = const_cast<double*>(_vertices);
  cells = const_cast<int*>(_cells);
  verticesRef = const_cast<double*>(_verticesRef);
  quadPts = const_cast<double*>(_quadPts);
  quadWts = const_cast<double*>(_quadWts);
  basis = const_cast<double*>(_basis);
  basisDerivRef = const_cast<double*>(_basisDerivRef);
  fieldTpdt = const_cast<double*>(_fieldTpdt);
  fieldT = const_cast<double*>(_fieldT);
  fieldTmdt = const_cast<double*>(_fieldTmdt);
  valsResidual = const_cast<double*>(_valsResidual);
  valsJacobian = const_cast<double*>(_valsJacobian);
} // constructor

pylith::feassemble::ElasticityImplicitData2DLinear::~ElasticityImplicitData2DLinear(void)
{}


// End of file
