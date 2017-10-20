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

#include "pylith/materials/IsotropicLinearIncompElasticityPlaneStrain.hh" // implementation of object methods

#include "pylith/materials/AuxiliaryFactory.hh" // USES AuxiliaryFactory

#include "pylith/topology/Field.hh" // USES Field::SubfieldInfo
#include "pylith/topology/FieldQuery.hh" // USES FieldQuery
#include "spatialdata/units/Nondimensional.hh" // USES Nondimensional

#include "pylith/materials/Query.hh" // USES Query
extern "C" {
#include "pylith/fekernels/dispvel.h" // USES DispVel kernels
#include "pylith/fekernels/pressure.h" // USES Pressure kernels
#include "pylith/fekernels/elasticity.h" // USES Elasticity kernels
#include "pylith/fekernels/linearincompelasticityplanestrain.h" // USES IsotropicLinearIncompElasticityPlaneStrain kernels
}

#include "pylith/utils/journals.hh" // USES PYLITH_COMPONENT_*

#include "spatialdata/spatialdb/GravityField.hh" // USES GravityField

#include "petscds.h"

// ----------------------------------------------------------------------
const char* pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::_pyreComponent = "isotropiclinearincompelasticityplanestrain";

// ----------------------------------------------------------------------
// Default constructor.
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::IsotropicLinearIncompElasticityPlaneStrain(void) :
    pylith::materials::MaterialNew(2),
    _useInertia(false),
    _useBodyForce(false),
    _useReferenceState(false)
{ // constructor
    pylith::utils::PyreComponent::name(_pyreComponent);
} // constructor

// ----------------------------------------------------------------------
// Destructor.
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::~IsotropicLinearIncompElasticityPlaneStrain(void) {} // destructor

// ----------------------------------------------------------------------
// Include inertia?
void
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::useInertia(const bool value) {
    PYLITH_COMPONENT_DEBUG("useInertia(value="<<value<<")");
    _useInertia = value;
} // useInertia


// ----------------------------------------------------------------------
// Include inertia?
bool
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::useInertia(void) const {
    return _useInertia;
} // useInertia


// ----------------------------------------------------------------------
// Include body force?
void
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::useBodyForce(const bool value) {
    PYLITH_COMPONENT_DEBUG("useBodyForce(value="<<value<<")");

    _useBodyForce = value;
} // useBodyForce


// ----------------------------------------------------------------------
// Include body force?
bool
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::useBodyForce(void) const {
    return _useBodyForce;
} // useBodyForce


// ----------------------------------------------------------------------
// Use reference stress and strain in computation of stress and
// strain?
void
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::useReferenceState(const bool value) {
    PYLITH_COMPONENT_DEBUG("useReferenceState="<<value<<")");

    _useReferenceState = value;
} // useReferenceState


// ----------------------------------------------------------------------
// Use reference stress and strain in computation of stress and
// strain?
bool
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::useReferenceState(void) const {
    return _useReferenceState;
} // useReferenceState


// ----------------------------------------------------------------------
// Verify configuration is acceptable.
void
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::verifyConfiguration(const pylith::topology::Field& solution) const {
    PYLITH_METHOD_BEGIN;
    PYLITH_COMPONENT_DEBUG("verifyConfiguration(solution="<<solution.label()<<")");

    // Verify solution contains expected fields.
    if (!solution.hasSubfield("displacement")) {
        throw std::runtime_error("Cannot find 'displacement' field in solution; required for material 'IsotropicLinearIncompElasticityPlaneStrain'.");
    } // if
    if (!solution.hasSubfield("pressure")) {
        throw std::runtime_error("Cannot find 'pressure' field in solution; required for material 'IsotropicLinearIncompElasticityPlaneStrain'.");
    } // if
    if (_useInertia && !solution.hasSubfield("velocity")) {
        throw std::runtime_error("Cannot find 'velocity' field in solution; required for material 'IsotropicLinearIncompElasticityPlaneStrain' with inertia.");
    } // if

    PYLITH_METHOD_END;
} // verifyConfiguration


// ----------------------------------------------------------------------
// Preinitialize material. Set names/sizes of auxiliary subfields.
void
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::_auxFieldSetup(void) {
    PYLITH_METHOD_BEGIN;
    PYLITH_COMPONENT_DEBUG("_auxFieldSetup()");

    const int dim = 2;

    assert(_auxMaterialFactory);
    assert(_normalizer);
    _auxMaterialFactory->initialize(_auxField, *_normalizer, dim);

    // :ATTENTION: The order for adding subfields must match the order of the auxiliary fields in the FE kernels.

    _auxMaterialFactory->density();
    _auxMaterialFactory->shearModulus();
    _auxMaterialFactory->bulkModulus();
    if (_gravityField) {
        _auxMaterialFactory->gravityField(_gravityField);
    } // if
    if (_useBodyForce) {
        _auxMaterialFactory->bodyForce();
    } // if
    if (_useReferenceState) {
        _auxMaterialFactory->referenceStress();
        _auxMaterialFactory->referenceStrain();
    } // if

    PYLITH_METHOD_END;
} // _auxFieldSetup

// ----------------------------------------------------------------------
// Set kernels for RHS residual G(t,s).
void
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::_setFEKernelsRHSResidual(const topology::Field& solution) const {
    PYLITH_METHOD_BEGIN;
    PYLITH_COMPONENT_DEBUG("_setFEKernelsRHSResidual(solution="<<solution.label()<<")");

    const PetscDM dm = solution.dmMesh(); assert(dm);
    PetscDS prob = NULL;
    PetscErrorCode err = DMGetDS(dm, &prob); PYLITH_CHECK_ERROR(err);

    const PetscInt i_disp = solution.subfieldInfo("displacement").index;
    const PetscInt i_pres = solution.subfieldInfo("pressure").index;

#if 1
    PYLITH_COMPONENT_ERROR(":TODO: @charles Implement gravbodyforce, grav, and bodyforce kernels.");
#else
    if (!solution.hasSubfield("velocity")) {
        // Displacement
        const PetscPointFunc g0u = (_gravityField && _useBodyForce) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_g0v_gravbodyforce :
                                   (_gravityField) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_g0v_grav :
                                   (_useBodyForce) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_g0v_bodyforce :
                                   NULL;
        const PetscPointFunc g1u = (!_useReferenceState) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_g1v : pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_g1v_refstate;

        err = PetscDSSetResidual(prob, i_disp, g0u, g1u); PYLITH_CHECK_ERROR(err);

        // Pressure
        const PetscPointFunc g0p = pylith_fekernels_Pressure_g0p;
        const PetscPointFunc g1p = NULL;
        err = PetscDSSetResidual(prob, i_pres, g0p, g1p); PYLITH_CHECK_ERROR(err);

    } else {
        const PetscInt i_vel = solution.subfieldInfo("velocity").index;

        // Displacement
        const PetscPointFunc g0u = pylith_fekernels_DispVel_g0u;
        const PetscPointFunc g1u = NULL;

        // Pressure
        const PetscPointFunc g0p = pylith_fekernels_Pressure_g0p;
        const PetscPointFunc g1p = NULL;

        // Velocity
        const PetscPointFunc g0v = (_gravityField && _useBodyForce) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_g0v_gravbodyforce :
                                   (_gravityField) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_g0v_grav :
                                   (_useBodyForce) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_g0v_bodyforce :
                                   NULL;
        const PetscPointFunc g1v = (!_useReferenceState) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_g1v : pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_g1v_refstate;

        err = PetscDSSetResidual(prob, i_disp, g0u, g1u); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetResidual(prob, i_vel,  g0v, g1v); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetResidual(prob, i_pres, g0p, g1p); PYLITH_CHECK_ERROR(err);
    } // if/else
#endif

    PYLITH_METHOD_END;
} // _setFEKernelsRHSResidual


// ----------------------------------------------------------------------
// Set kernels for RHS Jacobian G(t,s).
void
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::_setFEKernelsRHSJacobian(const topology::Field& solution) const {
    PYLITH_METHOD_BEGIN;
    PYLITH_COMPONENT_DEBUG("_setFEKernelsRHSJacobian(solution="<<solution.label()<<")");

    const PetscDM dm = solution.dmMesh(); assert(dm);
    PetscDS prob = NULL;
    PetscErrorCode err = DMGetDS(dm, &prob); PYLITH_CHECK_ERROR(err);

    const PetscInt i_disp = solution.subfieldInfo("displacement").index;
    const PetscInt i_pres = solution.subfieldInfo("pressure").index;

    if (!solution.hasSubfield("velocity")) {
        // Jacobian kernels
        const PetscPointJac Jg0uu = NULL;
        const PetscPointJac Jg1uu = NULL;
        const PetscPointJac Jg2uu = NULL;
        const PetscPointJac Jg3uu = pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_Jg3vu;

        const PetscPointJac Jg0up = NULL;
        const PetscPointJac Jg1up = NULL;
        const PetscPointJac Jg2up = NULL;
        const PetscPointJac Jg3up = NULL;

        const PetscPointJac Jg0pu = NULL;
        const PetscPointJac Jg1pu = pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_Jg1pu;
        const PetscPointJac Jg2pu = NULL;
        const PetscPointJac Jg3pu = NULL;

        const PetscPointJac Jg0pp = pylith_fekernels_Pressure_Jg0pp;
        const PetscPointJac Jg1pp = NULL;
        const PetscPointJac Jg2pp = NULL;
        const PetscPointJac Jg3pp = NULL;

        err = PetscDSSetJacobian(prob, i_disp, i_disp, Jg0uu, Jg1uu, Jg2uu, Jg3uu); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_disp, i_pres, Jg0up, Jg1up, Jg2up, Jg3up); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_pres, i_disp, Jg0pu, Jg1pu, Jg2pu, Jg3pu); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_pres, i_pres, Jg0pp, Jg1pp, Jg2pp, Jg3pp); PYLITH_CHECK_ERROR(err);

    } else {
        const PetscInt i_vel = solution.subfieldInfo("velocity").index;

        const PetscPointJac Jg0uu = NULL;
        const PetscPointJac Jg1uu = NULL;
        const PetscPointJac Jg2uu = NULL;
        const PetscPointJac Jg3uu = NULL;

        const PetscPointJac Jg0uv = pylith_fekernels_DispVel_Jg0uv;
        const PetscPointJac Jg1uv = NULL;
        const PetscPointJac Jg2uv = NULL;
        const PetscPointJac Jg3uv = NULL;

        const PetscPointJac Jg0vu = NULL;
        const PetscPointJac Jg1vu = NULL;
        const PetscPointJac Jg2vu = NULL;
        const PetscPointJac Jg3vu = pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_Jg3vu;

        const PetscPointJac Jg0vv = NULL;
        const PetscPointJac Jg1vv = NULL;
        const PetscPointJac Jg2vv = NULL;
        const PetscPointJac Jg3vv = NULL;

        const PetscPointJac Jg0up = NULL;
        const PetscPointJac Jg1up = NULL;
        const PetscPointJac Jg2up = NULL;
        const PetscPointJac Jg3up = NULL;

        const PetscPointJac Jg0vp = NULL;
        const PetscPointJac Jg1vp = NULL;
        const PetscPointJac Jg2vp = pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_Jg2vp;
        const PetscPointJac Jg3vp = NULL;

        const PetscPointJac Jg0pu = NULL;
        const PetscPointJac Jg1pu = pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_Jg1pu;
        const PetscPointJac Jg2pu = NULL;
        const PetscPointJac Jg3pu = NULL;

        const PetscPointJac Jg0pv = NULL;
        const PetscPointJac Jg1pv = NULL;
        const PetscPointJac Jg2pv = NULL;
        const PetscPointJac Jg3pv = NULL;

        const PetscPointJac Jg0pp = pylith_fekernels_Pressure_Jg0pp;
        const PetscPointJac Jg1pp = NULL;
        const PetscPointJac Jg2pp = NULL;
        const PetscPointJac Jg3pp = NULL;

        err = PetscDSSetJacobian(prob, i_disp, i_disp, Jg0uu, Jg1uu, Jg2uu, Jg3uu); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_disp, i_vel, Jg0uv, Jg1uv, Jg2uv, Jg3uv); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_vel, i_disp, Jg0vu, Jg1vu, Jg2vu, Jg3vu); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_vel, i_vel, Jg0vv, Jg1vv, Jg2vv, Jg3vv); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_disp, i_pres, Jg0up, Jg1up, Jg2up, Jg3up); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_pres, i_disp, Jg0pu, Jg1pu, Jg2pu, Jg3pu); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob,  i_vel, i_pres, Jg0vp, Jg1vp, Jg2vp, Jg3vp); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_pres,  i_vel, Jg0pv, Jg1pv, Jg2pv, Jg3pv); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_pres, i_pres, Jg0pp, Jg1pp, Jg2pp, Jg3pp); PYLITH_CHECK_ERROR(err);

    } // if/else

    PYLITH_METHOD_END;
} // _setFEKernelsRHSJacobian


// ----------------------------------------------------------------------
// Set kernels for LHS residual F(t,s,\dot{s}).
void
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::_setFEKernelsLHSResidual(const topology::Field& solution) const {
    PYLITH_METHOD_BEGIN;
    PYLITH_COMPONENT_DEBUG("_setFEKernelsLHSResidual(solution="<<solution.label()<<")");

    const PetscDM dm = solution.dmMesh(); assert(dm);
    PetscDS prob = NULL;
    PetscErrorCode err = DMGetDS(dm, &prob); PYLITH_CHECK_ERROR(err);

    const PetscInt i_disp = solution.subfieldInfo("displacement").index;
    const PetscInt i_pres = solution.subfieldInfo("pressure").index;

    if (!solution.hasSubfield("velocity")) {
        // F(t,s,\dot{s}) = \vec{0}.
    } else {

        const PetscInt i_vel = solution.subfieldInfo("velocity").index;

        // Displacement
        const PetscPointFunc f0u = pylith_fekernels_DispVel_f0u;
        const PetscPointFunc f1u = NULL;

        // Velocity
        const PetscPointFunc f0v = (_useInertia) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_f0v : NULL;
        const PetscPointFunc f1v = NULL;

        // Pressure
        const PetscPointFunc f0p = NULL;
        const PetscPointFunc f1p = NULL;

        err = PetscDSSetResidual(prob, i_disp, f0u, f1u); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetResidual(prob, i_vel,  f0v, f1v); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetResidual(prob, i_pres, f0p, f1p); PYLITH_CHECK_ERROR(err);

    } // if/else

    PYLITH_METHOD_END;
} // _setFEKernelsLHSResidual


// ----------------------------------------------------------------------
// Set kernels for LHS Jacobian F(t,s,\dot{s}).
void
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::_setFEKernelsLHSJacobianImplicit(const topology::Field& solution) const {
    PYLITH_METHOD_BEGIN;
    PYLITH_COMPONENT_DEBUG("_setFEKernelsLHSJacobianImplicit(solution="<<solution.label()<<")");

    const PetscDM dm = solution.dmMesh(); assert(dm);
    PetscDS prob = NULL;
    PetscErrorCode err = DMGetDS(dm, &prob); PYLITH_CHECK_ERROR(err);

    const PetscInt i_disp = solution.subfieldInfo("displacement").index;
    const PetscInt i_pres = solution.subfieldInfo("pressure").index;

    if (!solution.hasSubfield("velocity")) {
        // Jacobian kernels
        const PetscPointJac Jf0uu = pylith_fekernels_DispVel_Jf0uu_zero;
        const PetscPointJac Jf1uu = NULL;
        const PetscPointJac Jf2uu = NULL;
        const PetscPointJac Jf3uu = NULL;

        const PetscPointJac Jf0up = NULL;
        const PetscPointJac Jf1up = NULL;
        const PetscPointJac Jf2up = NULL;
        const PetscPointJac Jf3up = NULL;

        const PetscPointJac Jf0pu = NULL;
        const PetscPointJac Jf1pu = NULL;
        const PetscPointJac Jf2pu = NULL;
        const PetscPointJac Jf3pu = NULL;

        const PetscPointJac Jf0pp = NULL;
        const PetscPointJac Jf1pp = NULL;
        const PetscPointJac Jf2pp = NULL;
        const PetscPointJac Jf3pp = NULL;
        err = PetscDSSetJacobian(prob, i_disp, i_disp, Jf0uu, Jf1uu, Jf2uu, Jf3uu); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_disp, i_pres, Jf0up, Jf1up, Jf2up, Jf3up); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_pres, i_disp, Jf0pu, Jf1pu, Jf2pu, Jf3pu); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_pres, i_pres, Jf0pp, Jf1pp, Jf2pp, Jf3pp); PYLITH_CHECK_ERROR(err);
    } else {
        const PetscInt i_vel = solution.subfieldInfo("velocity").index;

        const PetscPointJac Jf0uu = pylith_fekernels_DispVel_Jf0uu_implicit;
        const PetscPointJac Jf1uu = NULL;
        const PetscPointJac Jf2uu = NULL;
        const PetscPointJac Jf3uu = NULL;

        const PetscPointJac Jf0uv = NULL;
        const PetscPointJac Jf1uv = NULL;
        const PetscPointJac Jf2uv = NULL;
        const PetscPointJac Jf3uv = NULL;

        const PetscPointJac Jf0vu = NULL;
        const PetscPointJac Jf1vu = NULL;
        const PetscPointJac Jf2vu = NULL;
        const PetscPointJac Jf3vu = NULL;

        const PetscPointJac Jf0vv = (_useInertia) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_Jf0vv_implicit : NULL;
        const PetscPointJac Jf1vv = NULL;
        const PetscPointJac Jf2vv = NULL;
        const PetscPointJac Jf3vv = NULL;

        const PetscPointJac Jf0up = NULL;
        const PetscPointJac Jf1up = NULL;
        const PetscPointJac Jf2up = NULL;
        const PetscPointJac Jf3up = NULL;

        const PetscPointJac Jf0pu = NULL;
        const PetscPointJac Jf1pu = NULL;
        const PetscPointJac Jf2pu = NULL;
        const PetscPointJac Jf3pu = NULL;

        const PetscPointJac Jf0vp = NULL;
        const PetscPointJac Jf1vp = NULL;
        const PetscPointJac Jf2vp = NULL;
        const PetscPointJac Jf3vp = NULL;

        const PetscPointJac Jf0pv = NULL;
        const PetscPointJac Jf1pv = NULL;
        const PetscPointJac Jf2pv = NULL;
        const PetscPointJac Jf3pv = NULL;

        const PetscPointJac Jf0pp = NULL;
        const PetscPointJac Jf1pp = NULL;
        const PetscPointJac Jf2pp = NULL;
        const PetscPointJac Jf3pp = NULL;

        err = PetscDSSetJacobian(prob, i_disp, i_disp, Jf0uu, Jf1uu, Jf2uu, Jf3uu); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_disp, i_vel,  Jf0uv, Jf1uv, Jf2uv, Jf3uv); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_disp, i_pres, Jf0up, Jf1up, Jf2up, Jf3up); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_vel,  i_disp, Jf0vu, Jf1vu, Jf2vu, Jf3vu); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_vel,  i_vel,  Jf0vv, Jf1vv, Jf2vv, Jf3vv); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_vel,  i_pres, Jf0vp, Jf1vp, Jf2vp, Jf3vp); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_pres, i_disp, Jf0pu, Jf1pu, Jf2pu, Jf3pu); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_pres, i_vel,  Jf0pv, Jf1pv, Jf2pv, Jf3pv); PYLITH_CHECK_ERROR(err);
        err = PetscDSSetJacobian(prob, i_pres, i_pres, Jf0pp, Jf1pp, Jf2pp, Jf3pp); PYLITH_CHECK_ERROR(err);
    } // if/else

    PYLITH_METHOD_END;
} // _setFEKernelsLHSJacobianImplicit


// ----------------------------------------------------------------------
// Set kernels for LHS Jacobian F(t,s,\dot{s}).
void
pylith::materials::IsotropicLinearIncompElasticityPlaneStrain::_setFEKernelsLHSJacobianExplicit(const topology::Field& solution) const {
    PYLITH_METHOD_BEGIN;
    PYLITH_COMPONENT_DEBUG("_setFEKernelsLHSJacobianExplicit(solution="<<solution.label()<<")");

    const PetscInt i_disp = solution.subfieldInfo("displacement").index;
    const PetscInt i_vel = solution.subfieldInfo("velocity").index;
    const PetscInt i_pres = solution.subfieldInfo("pressure").index;

    // Jacobian kernels
    const PetscPointJac Jf0uu = pylith_fekernels_DispVel_Jf0uu_explicit;
    const PetscPointJac Jf1uu = NULL;
    const PetscPointJac Jf2uu = NULL;
    const PetscPointJac Jf3uu = NULL;

    const PetscPointJac Jf0uv = NULL;
    const PetscPointJac Jf1uv = NULL;
    const PetscPointJac Jf2uv = NULL;
    const PetscPointJac Jf3uv = NULL;

    const PetscPointJac Jf0up = NULL;
    const PetscPointJac Jf1up = NULL;
    const PetscPointJac Jf2up = NULL;
    const PetscPointJac Jf3up = NULL;

    const PetscPointJac Jf0vu = NULL;
    const PetscPointJac Jf1vu = NULL;
    const PetscPointJac Jf2vu = NULL;
    const PetscPointJac Jf3vu = NULL;

    const PetscPointJac Jf0vv = (_useInertia) ? pylith_fekernels_IsotropicLinearIncompElasticityPlaneStrain_Jf0vv_explicit : NULL;
    const PetscPointJac Jf1vv = NULL;
    const PetscPointJac Jf2vv = NULL;
    const PetscPointJac Jf3vv = NULL;

    const PetscPointJac Jf0vp = NULL;
    const PetscPointJac Jf1vp = NULL;
    const PetscPointJac Jf2vp = NULL;
    const PetscPointJac Jf3vp = NULL;

    const PetscPointJac Jf0pu = NULL;
    const PetscPointJac Jf1pu = NULL;
    const PetscPointJac Jf2pu = NULL;
    const PetscPointJac Jf3pu = NULL;

    const PetscPointJac Jf0pv = NULL;
    const PetscPointJac Jf1pv = NULL;
    const PetscPointJac Jf2pv = NULL;
    const PetscPointJac Jf3pv = NULL;

    const PetscPointJac Jf0pp = NULL;
    const PetscPointJac Jf1pp = NULL;
    const PetscPointJac Jf2pp = NULL;
    const PetscPointJac Jf3pp = NULL;

    const PetscDM dm = solution.dmMesh(); assert(dm);
    PetscDS prob = NULL;
    PetscErrorCode err = DMGetDS(dm, &prob); PYLITH_CHECK_ERROR(err);
    err = PetscDSSetJacobian(prob, i_disp, i_disp, Jf0uu, Jf1uu, Jf2uu, Jf3uu); PYLITH_CHECK_ERROR(err);
    err = PetscDSSetJacobian(prob, i_disp, i_vel,  Jf0uv, Jf1uv, Jf2uv, Jf3uv); PYLITH_CHECK_ERROR(err);
    err = PetscDSSetJacobian(prob, i_disp, i_pres, Jf0up, Jf1up, Jf2up, Jf3up); PYLITH_CHECK_ERROR(err);
    err = PetscDSSetJacobian(prob, i_vel,  i_disp, Jf0vu, Jf1vu, Jf2vu, Jf3vu); PYLITH_CHECK_ERROR(err);
    err = PetscDSSetJacobian(prob, i_vel,  i_vel,  Jf0vv, Jf1vv, Jf2vv, Jf3vv); PYLITH_CHECK_ERROR(err);
    err = PetscDSSetJacobian(prob, i_vel,  i_pres, Jf0vp, Jf1vp, Jf2vp, Jf3vp); PYLITH_CHECK_ERROR(err);
    err = PetscDSSetJacobian(prob, i_pres, i_disp, Jf0pu, Jf1pu, Jf2pu, Jf3pu); PYLITH_CHECK_ERROR(err);
    err = PetscDSSetJacobian(prob, i_pres, i_vel,  Jf0pv, Jf1pv, Jf2pv, Jf3pv); PYLITH_CHECK_ERROR(err);
    err = PetscDSSetJacobian(prob, i_pres, i_pres, Jf0pp, Jf1pp, Jf2pp, Jf3pp); PYLITH_CHECK_ERROR(err);

    PYLITH_METHOD_END;
} // _setFEKernelsLHSJacobianExplicit


// End of file
