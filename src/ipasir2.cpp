/**
 * @file ipasir2cadical.cc
 * @author Markus Iser (markus.iser@kit.edu)
 * @brief Wrap CaDiCaL solver into IPASIR 2 solver
 * @version 0.1
 * @date 2022-11-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ipasir2.h"
#include "ccadical.h"

#include <cassert>

#include <cstring>
#include <iostream>
using std::string;

#include "config.hpp"
#include "options.hpp"


// Configuration

IPASIR_API ipasir2_errorcode ipasir2_options(void* S, ipasir2_option const** result) {
    int n_extra = 2;    
    ipasir2_option* extra = new ipasir2_option[n_extra];
    extra[0] = { "ipasir.limits.decisions", ipasir2_option_type::INT, -1, INT32_MAX };
    extra[1] = { "ipasir.limits.conflicts", ipasir2_option_type::INT, -1, INT32_MAX };

    ipasir2_option* solver_options = new ipasir2_option[CaDiCaL::number_of_options + n_extra + 1];
    int i = 0;
    for (; i < n_extra; ++i) {
        solver_options[i] = extra[i];
    }

    for (CaDiCaL::Option* option = CaDiCaL::Options::begin(); option != CaDiCaL::Options::end(); ++option) {
        if (option->optimizable) {
            solver_options[i].name = option->name;
            solver_options[i].type = ipasir2_option_type::INT;
            solver_options[i].min._int = option->lo;
            solver_options[i].max._int = option->hi;
            ++i;
        }
    }

    solver_options[i] = { 0 };

    *result = solver_options;
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_set_option(void* solver, char const* name, ipasir2_option_value value) {
    if (ccadical_has_option((CCaDiCaL*)solver, name)) {
        ccadical_set_option((CCaDiCaL*)solver, name, value._int);
        return IPASIR_E_OK;
    } else {
        if (!strcmp(name, "ipasir.limits.decisions")) {
            ccadical_limit((CCaDiCaL*)solver, "decisions", value._int);
            return IPASIR_E_OK;
        } 
        else if (!strcmp(name, "ipasir.limits.conflicts")) {
            ccadical_limit((CCaDiCaL*)solver, "conflicts", value._int);
            return IPASIR_E_OK;
        }
        else {
            return IPASIR_E_OPTION_UNKNOWN;
        }
    }
}


// Experimental

IPASIR_API ipasir2_errorcode ipasir2_assignment_size(void* solver, int32_t* result) {
    *result = ccadical_assignment_size((CCaDiCaL*)solver);
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_assignment(void* solver, int32_t index, int32_t* result) {
    *result = ccadical_assignment((CCaDiCaL*)solver, index);
    return IPASIR_E_OK;
}


// Basic IPASIR

IPASIR_API ipasir2_errorcode ipasir2_signature(char const** result) {
    *result = ccadical_signature();
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_init(void** result) {
    *result = ccadical_init();
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_release(void* solver) {
    ccadical_release((CCaDiCaL*)solver);
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_add(void* solver, int32_t lit_or_zero) {
    ccadical_add((CCaDiCaL*)solver, lit_or_zero);
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_assume(void* solver, int32_t lit) {
    ccadical_assume((CCaDiCaL*)solver, lit);
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_solve(void* solver, int* result) {
    *result = ccadical_solve((CCaDiCaL*)solver);
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_val(void* solver, int32_t lit, int32_t* result) {
    *result = ccadical_val((CCaDiCaL*)solver, lit);
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_failed(void* solver, int32_t lit, int* result) {
    *result = ccadical_failed((CCaDiCaL*)solver, lit);
    return IPASIR_E_OK;
}


// Callbacks

IPASIR_API ipasir2_errorcode ipasir2_set_terminate(void* solver, void* data, int (*terminate)(void* data)) {
    ccadical_set_terminate((CCaDiCaL*)solver, data, terminate);
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_set_learn(void* solver, void* data, void (*callback)(void* data, int32_t* clause)) {
    ccadical_set_learn((CCaDiCaL*)solver, data, INT32_MAX, callback);
    return IPASIR_E_OK;
}

IPASIR_API ipasir2_errorcode ipasir2_set_import_redundant_clause(void* solver, void* data, 
  void (*callback)(void* solver, int32_t** literals)) {
    return IPASIR_E_UNSUPPORTED;
}
