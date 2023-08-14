/**
 * @file ipasir2cadical.cc
 * @author Markus Iser (markus.iser@kit.edu)
 * @brief Wrap CaDiCaL solver into IPASIR 2 solver
 * @version 0.1
 * @date 2022-11-17
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

ipasir2_errorcode ipasir2_options(void* S, ipasir2_option const** result) {
    int n_extra = 2;    
    ipasir2_option* extra = new ipasir2_option[n_extra];
    extra[0] = { "ipasir.limits.decisions", -1, INT32_MAX, IPASIR2_S_INPUT, false };
    extra[1] = { "ipasir.limits.conflicts", -1, INT32_MAX, IPASIR2_S_INPUT, false };

    ipasir2_option* solver_options = new ipasir2_option[CaDiCaL::number_of_options + n_extra + 1];
    int i = 0;
    for (; i < n_extra; ++i) {
        solver_options[i] = extra[i];
    }

    for (CaDiCaL::Option* option = CaDiCaL::Options::begin(); option != CaDiCaL::Options::end(); ++option) {
        if (option->optimizable) {
            solver_options[i].name = option->name;
            solver_options[i].min = option->lo;
            solver_options[i].max = option->hi;
            solver_options[i].max_state = IPASIR2_S_CONFIG; // TODO: figure out eligible states and better default. conservative for now.
            solver_options[i].tunable = option->optimizable;
            solver_options[i].indexed = false;
            ++i;
        }
    }

    solver_options[i] = { 0 };

    *result = solver_options;
    return IPASIR2_E_OK;
}

ipasir2_errorcode ipasir2_set_option(void* solver, char const* name, int64_t index, int64_t value) {
    if (ccadical_has_option((CCaDiCaL*)solver, name)) {
        ccadical_set_option((CCaDiCaL*)solver, name, value);
        return IPASIR2_E_OK;
    } else {
        if (!strcmp(name, "ipasir.limits.decisions")) {
            ccadical_limit((CCaDiCaL*)solver, "decisions", value);
            return IPASIR2_E_OK;
        } 
        else if (!strcmp(name, "ipasir.limits.conflicts")) {
            ccadical_limit((CCaDiCaL*)solver, "conflicts", value);
            return IPASIR2_E_OK;
        }
        else {
            return IPASIR2_E_OPTION_UNKNOWN;
        }
    }
}


// Basic IPASIR

ipasir2_errorcode ipasir2_signature(char const** result) {
    *result = ccadical_signature();
    return IPASIR2_E_OK;
}

ipasir2_errorcode ipasir2_init(void** result) {
    *result = ccadical_init();
    return IPASIR2_E_OK;
}

ipasir2_errorcode ipasir2_release(void* solver) {
    ccadical_release((CCaDiCaL*)solver);
    return IPASIR2_E_OK;
}

ipasir2_errorcode ipasir2_add(void* solver, int32_t lit_or_zero) {
    ccadical_add((CCaDiCaL*)solver, lit_or_zero);
    return IPASIR2_E_OK;
}

ipasir2_errorcode ipasir2_assume(void* solver, int32_t lit) {
    ccadical_assume((CCaDiCaL*)solver, lit);
    return IPASIR2_E_OK;
}

ipasir2_errorcode ipasir2_solve(void* solver, int* result) {
    *result = ccadical_solve((CCaDiCaL*)solver);
    return IPASIR2_E_OK;
}

ipasir2_errorcode ipasir2_val(void* solver, int32_t lit, int32_t* result) {
    *result = ccadical_val((CCaDiCaL*)solver, lit);
    return IPASIR2_E_OK;
}

ipasir2_errorcode ipasir2_failed(void* solver, int32_t lit, int* result) {
    *result = ccadical_failed((CCaDiCaL*)solver, lit);
    return IPASIR2_E_OK;
}


// Callbacks

ipasir2_errorcode ipasir2_set_terminate(void* solver, void* data, 
        int (*callback)(void* data)) {
    ccadical_set_terminate((CCaDiCaL*)solver, data, callback);
    return IPASIR2_E_OK;
}

ipasir2_errorcode ipasir2_set_export(void* solver, void* data, int32_t max_length,
        void (*callback)(void* data, int32_t const* clause)) {
    if (max_length < 0) {
        return IPASIR2_E_UNSUPPORTED_ARGUMENT;
    }
    ccadical_set_learn((CCaDiCaL*)solver, data, max_length, callback);
    return IPASIR2_E_OK;
}

ipasir2_errorcode ipasir2_set_import(void* solver, void* data, ipasir2_pledge pledge,
        int32_t const* (*import)(void* data)) {
    return IPASIR2_E_UNSUPPORTED;
}

ipasir2_errorcode ipasir2_set_notify(void* solver, void* data, 
        void (*notify)(void* data, int32_t const* assigned, int32_t const* unassigned)) {
    return IPASIR2_E_UNSUPPORTED;
}
