// -*- C++ -*-
//
// ----------------------------------------------------------------------
//
// Brad T. Aagaard, U.S. Geological Survey
// Charles A. Williams, GNS Science
// Matthew G. Knepley, University of Chicago
//
// This code was developed as part of the Computational Infrastructure
// for Geodynamics (http://geodynamics.org).
//
// Copyright (c) 2010-2015 University of California, Davis
//
// See COPYING for license information.
//
// ----------------------------------------------------------------------
//

#include <portinfo>

#include "TestIsotropicLinearElasticityPlaneStrain.hh" // Implementation of class methods

#include "pylith/materials/IsotropicLinearElasticityPlaneStrain.hh" // USES IsotropicLinearElasticityPlaneStrain
#include "pylith/materials/Query.hh" // USES Query

#include "pylith/topology/Field.hh" // USES Field
#include "pylith/topology/FieldQuery.hh" // USES FieldQuery
#include "pylith/utils/error.hh" // USES PYLITH_METHOD_BEGIN/END
#include "pylith/utils/journals.hh" // USES JournalingComponent

#include "spatialdata/spatialdb/SimpleDB.hh" // USES SimpleDB
#include "spatialdata/spatialdb/SimpleIOAscii.hh" // USES SimpleIOAscii

extern "C" {
#include "pylith/fekernels/dispvel.h" // USES DispVel kernels
#include "pylith/fekernels/elasticity.h" // USES Elasticity kernels
#include "pylith/fekernels/linearelasticityplanestrain.h" // USES IsotropicLinearElasticityPlaneStrain kernels
}


// ----------------------------------------------------------------------
// Setup testing data.
void
pylith::materials::TestIsotropicLinearElasticityPlaneStrain::setUp(void)
{ // setUp
    TestMaterialNew::setUp();
    _mymaterial = new IsotropicLinearElasticityPlaneStrain(); CPPUNIT_ASSERT(_mymaterial);
    _mydata = NULL;

    _mymaterial->PyreComponent::identifier("TestIsotropicLinearElasticityPlaneStrain");
    const char* journal = _mymaterial->PyreComponent::name();
    journal::debug_t debug(journal);
    debug.activate();
} // setUp


// ----------------------------------------------------------------------
// Deallocate testing data.
void
pylith::materials::TestIsotropicLinearElasticityPlaneStrain::tearDown(void)
{ // tearDown
    TestMaterialNew::tearDown();

    delete _mymaterial; _mymaterial = NULL;
    delete _mydata; _mydata = NULL;
} // tearDown


// ----------------------------------------------------------------------
// Test useInertia().
void
pylith::materials::TestIsotropicLinearElasticityPlaneStrain::testUseInertia(void)
{ // testUseInertia
    PYLITH_METHOD_BEGIN;

    CPPUNIT_ASSERT(_mymaterial);

    const bool flag = false; // default
    CPPUNIT_ASSERT_EQUAL(flag, _mymaterial->_useInertia);

    _mymaterial->useInertia(!flag);
    CPPUNIT_ASSERT_EQUAL(!flag, _mymaterial->_useInertia);

    PYLITH_METHOD_END;
} // testUseInertia


// ----------------------------------------------------------------------
// Test useBodyForce().
void
pylith::materials::TestIsotropicLinearElasticityPlaneStrain::testUseBodyForce(void)
{ // testUseBodyForce
    PYLITH_METHOD_BEGIN;

    CPPUNIT_ASSERT(_mymaterial);

    const bool flag = false; // default
    CPPUNIT_ASSERT_EQUAL(flag, _mymaterial->_useBodyForce);

    _mymaterial->useBodyForce(!flag);
    CPPUNIT_ASSERT_EQUAL(!flag, _mymaterial->_useBodyForce);

    PYLITH_METHOD_END;
} // testUseBodyForce


// ----------------------------------------------------------------------
// Test useReferenceState().
void
pylith::materials::TestIsotropicLinearElasticityPlaneStrain::testUseReferenceState(void)
{ // testUseReferenceState
    PYLITH_METHOD_BEGIN;

    CPPUNIT_ASSERT(_mymaterial);

    const bool flag = false; // default
    CPPUNIT_ASSERT_EQUAL(flag, _mymaterial->_useReferenceState);

    _mymaterial->useReferenceState(!flag);
    CPPUNIT_ASSERT_EQUAL(!flag, _mymaterial->_useReferenceState);

    PYLITH_METHOD_END;
} // testUseReferenceState


// ----------------------------------------------------------------------
// Test auxFieldsSetup().
void
pylith::materials::TestIsotropicLinearElasticityPlaneStrain::test_auxFieldsSetup(void)
{ // test_auxFieldsSetup
    PYLITH_METHOD_BEGIN;

    CPPUNIT_ASSERT(_mymaterial);
    CPPUNIT_ASSERT(_mesh);
    CPPUNIT_ASSERT(_mydata);

    delete _mymaterial->_auxFields; _mymaterial->_auxFields = new topology::Field(*_mesh); CPPUNIT_ASSERT(_mymaterial->_auxFields);
    delete _mymaterial->_auxFieldsQuery; _mymaterial->_auxFieldsQuery = new topology::FieldQuery(*_mymaterial->_auxFields); CPPUNIT_ASSERT(_mymaterial->_auxFieldsQuery);
    _mymaterial->_auxFieldsSetup();

    // Check discretizations
    { // density
        const char* label = "density";
        const pylith::topology::Field::SubfieldInfo& info = _mymaterial->_auxFields->subfieldInfo(label);
        CPPUNIT_ASSERT_EQUAL(1, info.numComponents);
        CPPUNIT_ASSERT_EQUAL(std::string(label), info.metadata.label);
        CPPUNIT_ASSERT_EQUAL(pylith::topology::Field::SCALAR, info.metadata.vectorFieldType);
        CPPUNIT_ASSERT_EQUAL(_mydata->densityScale, info.metadata.scale);
        CPPUNIT_ASSERT_EQUAL(false, info.metadata.dimsOkay);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.basisOrder);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.quadOrder);
        CPPUNIT_ASSERT_EQUAL(true, info.fe.isBasisContinuous);
    } // density

    { // shear modulus
        const char* label = "shear_modulus";
        const pylith::topology::Field::SubfieldInfo& info = _mymaterial->_auxFields->subfieldInfo(label);
        CPPUNIT_ASSERT_EQUAL(1, info.numComponents);
        CPPUNIT_ASSERT_EQUAL(std::string(label), info.metadata.label);
        CPPUNIT_ASSERT_EQUAL(pylith::topology::Field::SCALAR, info.metadata.vectorFieldType);
        CPPUNIT_ASSERT_EQUAL(_mydata->pressureScale, info.metadata.scale);
        CPPUNIT_ASSERT_EQUAL(false, info.metadata.dimsOkay);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.basisOrder);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.quadOrder);
        CPPUNIT_ASSERT_EQUAL(true, info.fe.isBasisContinuous);
    } // shear modulus

    { // bulk modulus
        const char* label = "bulk_modulus";
        const pylith::topology::Field::SubfieldInfo& info = _mymaterial->_auxFields->subfieldInfo(label);
        CPPUNIT_ASSERT_EQUAL(1, info.numComponents);
        CPPUNIT_ASSERT_EQUAL(std::string(label), info.metadata.label);
        CPPUNIT_ASSERT_EQUAL(pylith::topology::Field::SCALAR, info.metadata.vectorFieldType);
        CPPUNIT_ASSERT_EQUAL(_mydata->pressureScale, info.metadata.scale);
        CPPUNIT_ASSERT_EQUAL(false, info.metadata.dimsOkay);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.basisOrder);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.quadOrder);
        CPPUNIT_ASSERT_EQUAL(true, info.fe.isBasisContinuous);
    } // bulk modulus

    if (_mydata->useBodyForce) { // body force
        const char* label = "body_force";
        const PylithReal forceScale = _mydata->densityScale * _mydata->lengthScale / (_mydata->timeScale * _mydata->timeScale);
        const pylith::topology::Field::SubfieldInfo& info = _mymaterial->_auxFields->subfieldInfo(label);
        CPPUNIT_ASSERT_EQUAL(2, info.numComponents);
        CPPUNIT_ASSERT_EQUAL(std::string(label), info.metadata.label);
        CPPUNIT_ASSERT_EQUAL(pylith::topology::Field::VECTOR, info.metadata.vectorFieldType);
        CPPUNIT_ASSERT_EQUAL(forceScale, info.metadata.scale);
        CPPUNIT_ASSERT_EQUAL(false, info.metadata.dimsOkay);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.basisOrder);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.quadOrder);
        CPPUNIT_ASSERT_EQUAL(true, info.fe.isBasisContinuous);
    } // body force

    if (_mydata->useReferenceState) { // reference stress and strain
        const char* label = "reference_stress";
        const pylith::topology::Field::SubfieldInfo& info = _mymaterial->_auxFields->subfieldInfo(label);
        CPPUNIT_ASSERT_EQUAL(4, info.numComponents);
        CPPUNIT_ASSERT_EQUAL(std::string(label), info.metadata.label);
        CPPUNIT_ASSERT_EQUAL(pylith::topology::Field::OTHER, info.metadata.vectorFieldType);
        CPPUNIT_ASSERT_EQUAL(_mydata->pressureScale, info.metadata.scale);
        CPPUNIT_ASSERT_EQUAL(false, info.metadata.dimsOkay);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.basisOrder);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.quadOrder);
        CPPUNIT_ASSERT_EQUAL(true, info.fe.isBasisContinuous);
    } // reference stress

    if (_mydata->useReferenceState) { // referece stress and strain
        const char* label = "reference_strain";
        const pylith::topology::Field::SubfieldInfo& info = _mymaterial->_auxFields->subfieldInfo(label);
        CPPUNIT_ASSERT_EQUAL(4, info.numComponents);
        CPPUNIT_ASSERT_EQUAL(std::string(label), info.metadata.label);
        CPPUNIT_ASSERT_EQUAL(pylith::topology::Field::OTHER, info.metadata.vectorFieldType);
        CPPUNIT_ASSERT_EQUAL(1.0, info.metadata.scale);
        CPPUNIT_ASSERT_EQUAL(false, info.metadata.dimsOkay);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.basisOrder);
        CPPUNIT_ASSERT_EQUAL(-1, info.fe.quadOrder);
        CPPUNIT_ASSERT_EQUAL(true, info.fe.isBasisContinuous);
    } // reference strain

    // Make sure DB query functions are set correctly.
    CPPUNIT_ASSERT_EQUAL(&pylith::topology::FieldQuery::dbQueryGeneric, _mymaterial->_auxFieldsQuery->queryFn("density"));
    CPPUNIT_ASSERT_EQUAL(&pylith::materials::Query::dbQueryShearModulus, _mymaterial->_auxFieldsQuery->queryFn("shear_modulus"));
    CPPUNIT_ASSERT_EQUAL(&pylith::materials::Query::dbQueryBulkModulus, _mymaterial->_auxFieldsQuery->queryFn("bulk_modulus"));
    if (_mydata->useBodyForce) {
        CPPUNIT_ASSERT_EQUAL(&pylith::topology::FieldQuery::dbQueryGeneric, _mymaterial->_auxFieldsQuery->queryFn("body_force"));
    } // if
    if (_mydata->useReferenceState) {
        CPPUNIT_ASSERT_EQUAL(&pylith::topology::FieldQuery::dbQueryGeneric, _mymaterial->_auxFieldsQuery->queryFn("reference_stress"));
        CPPUNIT_ASSERT_EQUAL(&pylith::topology::FieldQuery::dbQueryGeneric, _mymaterial->_auxFieldsQuery->queryFn("reference_strain"));
    } // if

    PYLITH_METHOD_END;
} // test_auxFieldsSetup


// ----------------------------------------------------------------------
// Test getAuxField().
void
pylith::materials::TestIsotropicLinearElasticityPlaneStrain::testGetAuxField(void)
{ // testGetAuxField
    PYLITH_METHOD_BEGIN;

    _initializeFull();

    CPPUNIT_ASSERT(_mymaterial);
    CPPUNIT_ASSERT(_mesh);

    { // Test getting density field.
        pylith::topology::Field density(*_mesh);
        _mymaterial->getAuxField(&density, "density");

        density.createScatter(density.mesh()); // Populate global vector.
        density.scatterLocalToContext();

        //density.view("DENSITY"); // DEBUGGING

        // Check result
        CPPUNIT_ASSERT_EQUAL(std::string("density"), std::string(density.label()));
        CPPUNIT_ASSERT_EQUAL(_mydata->dimension, density.spaceDim());

        pylith::topology::FieldQuery queryDensity(density);
        queryDensity.queryFn("density", pylith::topology::FieldQuery::dbQueryGeneric);
        queryDensity.openDB(_auxDB, _mydata->lengthScale);

        PylithReal norm = 0.0;
        const PylithReal t = _mydata->t;
        const PetscDM dm = density.dmMesh(); CPPUNIT_ASSERT(dm);
        PetscErrorCode err = DMComputeL2Diff(dm, t, queryDensity.functions(), (void**)queryDensity.contextPtrs(), density.globalVector(), &norm); CPPUNIT_ASSERT(!err);
        queryDensity.closeDB(_auxDB);

        const PylithReal tolerance = 1.0e-6;
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, norm, tolerance);
    } // Test getting density field

    { // Test getting bulkModulus field.
        pylith::topology::Field bulkModulus(*_mesh);
        _mymaterial->getAuxField(&bulkModulus, "bulk_modulus");

        bulkModulus.createScatter(bulkModulus.mesh()); // Populate global vector.
        bulkModulus.scatterLocalToContext();

        //bulkModulus.view("BULK MODULUS"); // DEBUGGING

        // Check result
        CPPUNIT_ASSERT_EQUAL(std::string("bulk_modulus"), std::string(bulkModulus.label()));
        CPPUNIT_ASSERT_EQUAL(_mydata->dimension, bulkModulus.spaceDim());

        pylith::topology::FieldQuery queryBulkModulus(bulkModulus);
        queryBulkModulus.queryFn("bulk_modulus", pylith::materials::Query::dbQueryBulkModulus);
        queryBulkModulus.openDB(_auxDB, _mydata->lengthScale);

        PylithReal norm = 0.0;
        const PylithReal t = _mydata->t;
        const PetscDM dm = bulkModulus.dmMesh(); CPPUNIT_ASSERT(dm);
        PetscErrorCode err = DMComputeL2Diff(dm, t, queryBulkModulus.functions(), (void**)queryBulkModulus.contextPtrs(), bulkModulus.globalVector(), &norm); CPPUNIT_ASSERT(!err);
        queryBulkModulus.closeDB(_auxDB);

        const PylithReal tolerance = 1.0e-6;
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, norm, tolerance);
    } // Test getting bulkModulus field

    std::cout << " :TODO: Add test for getting reference stress/strain. " << std::endl;

    PYLITH_METHOD_END;
} // testGetAuxField


// ----------------------------------------------------------------------
// Get material.
pylith::materials::MaterialNew*
pylith::materials::TestIsotropicLinearElasticityPlaneStrain::_material(void)
{ // _material
    return _mymaterial;
} // _material


// ----------------------------------------------------------------------
// Get test data.
pylith::materials::TestMaterialNew_Data*
pylith::materials::TestIsotropicLinearElasticityPlaneStrain::_data(void)
{ // _data
    return _mydata;
} // _data


// ----------------------------------------------------------------------
// Setup and populate solution field.
void
pylith::materials::TestIsotropicLinearElasticityPlaneStrain::_setupSolutionField(pylith::topology::Field* field,
                                                                                 const char* dbFilename,
                                                                                 const bool isClone)
{ // _setupSolutionField
    CPPUNIT_ASSERT(field);
    CPPUNIT_ASSERT(dbFilename);

    // Create subfields.
    const bool isDot = strstr(field->label(), "dot") != NULL;
    std::string subfields[2];
    if (isDot) {
        subfields[0] = "displacement_dot";
        subfields[1] = "velocity_dot";
    } else {
        subfields[0] = "displacement";
        subfields[1] = "velocity";
    } // if/else

    if (!isClone) {
        CPPUNIT_ASSERT(_mydata->solnDiscretizations);
        const pylith::topology::Field::DiscretizeInfo& dispFEInfo = _mydata->solnDiscretizations[0];
        const pylith::topology::Field::DiscretizeInfo& velFEInfo = _mydata->solnDiscretizations[1];
        if (isDot) {
            const char* componentsDisp[2] = {"displacement_dot_x", "displacement_dot_y"};
            const char* componentsVel[2] = {"velocity_dot_x", "velocity_dot_y"};
            field->subfieldAdd(subfields[0].c_str(), componentsDisp, _mydata->dimension, topology::Field::VECTOR, dispFEInfo.basisOrder, dispFEInfo.quadOrder, dispFEInfo.isBasisContinuous, _mydata->lengthScale);
            field->subfieldAdd(subfields[1].c_str(), componentsVel, _mydata->dimension, topology::Field::VECTOR, velFEInfo.basisOrder, velFEInfo.quadOrder, velFEInfo.isBasisContinuous, _mydata->lengthScale / _mydata->timeScale);
        } else {
            const char* componentsDisp[2] = {"displacement_x", "displacement_y"};
            const char* componentsVel[2] = {"velocity_x", "velocity_y"};
            field->subfieldAdd(subfields[0].c_str(), componentsDisp, _mydata->dimension, topology::Field::VECTOR, dispFEInfo.basisOrder, dispFEInfo.quadOrder, dispFEInfo.isBasisContinuous, _mydata->lengthScale);
            field->subfieldAdd(subfields[1].c_str(), componentsVel, _mydata->dimension, topology::Field::VECTOR, velFEInfo.basisOrder, velFEInfo.quadOrder, velFEInfo.isBasisContinuous, _mydata->lengthScale / _mydata->timeScale);
        } // if/else
        field->subfieldsSetup();
    } // if
    field->allocate();
    field->zeroLocal();

    spatialdata::spatialdb::SimpleDB fieldDB;
    spatialdata::spatialdb::SimpleIOAscii dbIO;
    dbIO.filename(dbFilename);
    fieldDB.ioHandler(&dbIO);
    fieldDB.label("IsotropicLinearElasciticityPlaneStrain solution database");
    fieldDB.queryType(spatialdata::spatialdb::SimpleDB::LINEAR);

    pylith::topology::FieldQuery queryField(*field);
    queryField.queryFn(subfields[0].c_str(), pylith::topology::FieldQuery::dbQueryGeneric);
    queryField.queryFn(subfields[1].c_str(), pylith::topology::FieldQuery::dbQueryGeneric);
    queryField.openDB(&fieldDB, _mydata->lengthScale);
    queryField.queryDB();
    queryField.closeDB(&fieldDB);
} // _setupSolutionField

// ----------------------------------------------------------------------
// Constructor
pylith::materials::TestIsotropicLinearElasticityPlaneStrain_Data::TestIsotropicLinearElasticityPlaneStrain_Data(const bool withInertia,
                                                                                                                const bool withBodyForce,
                                                                                                                const bool withReferenceState) :
    useInertia(withInertia),
    useBodyForce(withBodyForce),
    useReferenceState(withReferenceState)
{ // constructor
    dimension = 2;
    numSolnFields = 2;

    // RHS Residual (nfields*2)
    // disp
    _kernelsRHSResidual[0*2+0] = pylith_fekernels_DispVel_g0u;
    _kernelsRHSResidual[0*2+1] = NULL;
    // vel
    _kernelsRHSResidual[1*2+0] = NULL;
    _kernelsRHSResidual[1*2+1] = pylith_fekernels_IsotropicLinearElasticityPlaneStrain_g1v;

    // RHS Jacobian (nfields*nfields*4)
    // disp/disp
    _kernelsRHSJacobian[(0*2+0)*4+0] = NULL;
    _kernelsRHSJacobian[(0*2+0)*4+1] = NULL;
    _kernelsRHSJacobian[(0*2+0)*4+2] = NULL;
    _kernelsRHSJacobian[(0*2+0)*4+3] = NULL;

    // disp/vel
    _kernelsRHSJacobian[(0*2+1)*4+0] = pylith_fekernels_DispVel_Jg0uv;
    _kernelsRHSJacobian[(0*2+1)*4+1] = NULL;
    _kernelsRHSJacobian[(0*2+1)*4+2] = NULL;
    _kernelsRHSJacobian[(0*2+1)*4+3] = NULL;

    // vel/disp
    _kernelsRHSJacobian[(1*2+0)*4+0] = NULL;
    _kernelsRHSJacobian[(1*2+0)*4+1] = NULL;
    _kernelsRHSJacobian[(1*2+0)*4+2] = NULL;
    _kernelsRHSJacobian[(1*2+0)*4+3] = pylith_fekernels_IsotropicLinearElasticityPlaneStrain_Jg3vu;

    // vel/vel
    _kernelsRHSJacobian[(1*2+1)*4+0] = NULL;
    _kernelsRHSJacobian[(1*2+1)*4+1] = NULL;
    _kernelsRHSJacobian[(1*2+1)*4+2] = NULL;
    _kernelsRHSJacobian[(1*2+1)*4+3] = NULL;

    // LHS Residual (nfields*2)
    // disp
    _kernelsLHSResidual[0*2+0] = pylith_fekernels_DispVel_f0u;
    _kernelsLHSResidual[0*2+1] = NULL;
    // vel
    _kernelsLHSResidual[1*2+0] = NULL;
    _kernelsLHSResidual[1*2+1] = NULL;

    // LHS Jacobian Implicit (nfields*nfields*4)
    // disp/disp
    _kernelsLHSJacobianImplicit[(0*2+0)*4+0] = pylith_fekernels_DispVel_Jf0uu_implicit;
    _kernelsLHSJacobianImplicit[(0*2+0)*4+1] = NULL;
    _kernelsLHSJacobianImplicit[(0*2+0)*4+2] = NULL;
    _kernelsLHSJacobianImplicit[(0*2+0)*4+3] = NULL;

    // disp/vel
    _kernelsLHSJacobianImplicit[(0*2+1)*4+0] = NULL;
    _kernelsLHSJacobianImplicit[(0*2+1)*4+1] = NULL;
    _kernelsLHSJacobianImplicit[(0*2+1)*4+2] = NULL;
    _kernelsLHSJacobianImplicit[(0*2+1)*4+3] = NULL;

    // vel/disp
    _kernelsLHSJacobianImplicit[(1*2+0)*4+0] = NULL;
    _kernelsLHSJacobianImplicit[(1*2+0)*4+1] = NULL;
    _kernelsLHSJacobianImplicit[(1*2+0)*4+2] = NULL;
    _kernelsLHSJacobianImplicit[(1*2+0)*4+3] = NULL;

    // vel/vel
    _kernelsLHSJacobianImplicit[(1*2+1)*4+0] = NULL;
    _kernelsLHSJacobianImplicit[(1*2+1)*4+1] = NULL;
    _kernelsLHSJacobianImplicit[(1*2+1)*4+2] = NULL;
    _kernelsLHSJacobianImplicit[(1*2+1)*4+3] = NULL;

    // LHS Jacobian Explicit (nfields*nfields*4)
    // disp/disp
    _kernelsLHSJacobianExplicit[(0*2+0)*4+0] = pylith_fekernels_DispVel_Jf0uu_explicit;
    _kernelsLHSJacobianExplicit[(0*2+0)*4+1] = NULL;
    _kernelsLHSJacobianExplicit[(0*2+0)*4+2] = NULL;
    _kernelsLHSJacobianExplicit[(0*2+0)*4+3] = NULL;

    // disp/vel
    _kernelsLHSJacobianExplicit[(0*2+1)*4+0] = NULL;
    _kernelsLHSJacobianExplicit[(0*2+1)*4+1] = NULL;
    _kernelsLHSJacobianExplicit[(0*2+1)*4+2] = NULL;
    _kernelsLHSJacobianExplicit[(0*2+1)*4+3] = NULL;

    // vel/disp
    _kernelsLHSJacobianExplicit[(1*2+0)*4+0] = NULL;
    _kernelsLHSJacobianExplicit[(1*2+0)*4+1] = NULL;
    _kernelsLHSJacobianExplicit[(1*2+0)*4+2] = NULL;
    _kernelsLHSJacobianExplicit[(1*2+0)*4+3] = NULL;

    // vel/vel
    _kernelsLHSJacobianExplicit[(1*2+1)*4+0] = NULL;
    _kernelsLHSJacobianExplicit[(1*2+1)*4+1] = NULL;
    _kernelsLHSJacobianExplicit[(1*2+1)*4+2] = NULL;
    _kernelsLHSJacobianExplicit[(1*2+1)*4+3] = NULL;

    kernelsRHSResidual = const_cast<PetscPointFunc*>(_kernelsRHSResidual);
    kernelsRHSJacobian = const_cast<PetscPointJac*>(_kernelsRHSJacobian);
    kernelsLHSResidual = const_cast<PetscPointFunc*>( _kernelsLHSResidual);
    kernelsLHSJacobianImplicit = const_cast<PetscPointJac*>(_kernelsLHSJacobianImplicit);
    kernelsLHSJacobianExplicit = const_cast<PetscPointJac*>( _kernelsLHSJacobianExplicit);
} // constructor


// ----------------------------------------------------------------------
// Destructor
pylith::materials::TestIsotropicLinearElasticityPlaneStrain_Data::~TestIsotropicLinearElasticityPlaneStrain_Data(void)
{ // destructor
} // destructor




// End of file