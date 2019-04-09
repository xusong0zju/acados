/*
 *    This file is part of acados.
 *
 *    acados is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    acados is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with acados; if not, write to the Free Software Foundation,
 *    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

// standard
#include <stdio.h>
#include <stdlib.h>
// acados
#include "acados/utils/print.h"
#include "acados_c/ocp_nlp_interface.h"
#include "acados_c/external_function_interface.h"
#include "acados/ocp_nlp/ocp_nlp_cost_ls.h"

// blasfeo
#include "blasfeo/include/blasfeo_d_aux.h"
#include "blasfeo/include/blasfeo_d_aux_ext_dep.h"

// example specific
#include "{{ ocp.model_name }}_model/{{ ocp.model_name }}_model.h"
{% if ocp.dims.npd > 0 %}
#include "{{ ocp.con_p_name }}_p_constraint/{{ ocp.con_p_name }}_p_constraint.h"
{% endif %}
{% if ocp.dims.nh > 0 %}
#include "{{ ocp.con_h_name }}_h_constraint/{{ ocp.con_h_name }}_h_constraint.h"
{% endif %}

#include "acados_solver_{{ocp.model_name}}.h"

{% for value, key in ocp.constants %}
#define {{ value }} {{ key }}
{% endfor %}
#define NX_   {{ ocp.dims.nx }}
#define NZ_   {{ ocp.dims.nz }}
#define NU_   {{ ocp.dims.nu }}
#define NP_   {{ ocp.dims.np }}
#define NBX_  {{ ocp.dims.nbx }}
#define NBU_  {{ ocp.dims.nbu }}
#define NG_   {{ ocp.dims.ng }}
#define NBXN_ {{ ocp.dims.nbxN }}
#define NGN_  {{ ocp.dims.ngN }}
#define NY_   {{ ocp.dims.ny }}
#define NYN_  {{ ocp.dims.nyN }}
#define N_    {{ ocp.dims.N }}
#define NPD_  {{ ocp.dims.npd }}
#define NPDN_ {{ ocp.dims.npdN }}
#define NH_   {{ ocp.dims.nh }}
#define NHN_  {{ ocp.dims.nhN }}

#if NX_ < 1
#define NX   1
#else
#define NX   NX_
#endif

#if NZ_ < 1
#define NZ   1
#else
#define NZ   NZ_
#endif

#if NU_ < 1
#define NU   1
#else
#define NU   NU_
#endif

#if NP_ < 1
#define NP   1
#else
#define NP   NP_
#endif

#if NBX_ < 1
#define NBX   1
#else
#define NBX   NBX_
#endif

#if NBU_ < 1
#define NBU   1
#else
#define NBU   NBU_
#endif

#if NG_ < 1
#define NG   1
#else
#define NG   NG_
#endif

#if NBXN_ < 1
#define NBXN   1
#else
#define NBXN   NBXN_
#endif

#if NGN_ < 1
#define NGN   1
#else
#define NGN  NGN_
#endif

#if NY_ < 1
#define NY   1
#else
#define NY   NY_
#endif

#if NYN_ < 1
#define NYN   1
#else
#define NYN   NYN_
#endif

#if N_ < 1
#define N   1
#else
#define N   N_
#endif

#if NPD_ < 1
#define NPD   1
#else
#define NPD   NPD_
#endif

#if NPDN_ < 1
#define NPDN   1
#else
#define NPDN   NPDN_
#endif

#if NH_ < 1
#define NH   1
#else
#define NH   NH_
#endif

#if NHN_ < 1
#define NHN   1
#else
#define NHN   NHN_
#endif

int acados_create() {

    int status = 0;

    double Tf = {{ ocp.solver_config.tf }};

    // set up bounds for stage 0 
    // u
    int idxbu0[NBU];
    {% for i in range(end=ocp.dims.nbu) %}
    idxbu0[{{i}}] = {{ocp.constraints.idxbu[i]}};
    {%- endfor %}
    double lbu0[NBU]; 
    double ubu0[NBU];
    {% for i in range(end=ocp.dims.nbu) %}
    lbu0[{{i}}] = {{ ocp.constraints.lbu[i] }};
    ubu0[{{i}}] = {{ ocp.constraints.ubu[i] }};
    {%- endfor %}
    
    // x
    int idxbx0[NX];
    {% for i in range(end=ocp.dims.nx) %}
    idxbx0[{{i}}] = {{i}};
    {%- endfor %}
    double lbx0[NX]; 
    double ubx0[NX];
    {% for i in range(end=ocp.dims.nx) %}
    lbx0[{{i}}] = {{ ocp.constraints.x0[i] }};
    ubx0[{{i}}] = {{ ocp.constraints.x0[i] }};
    {%- endfor %}


    // set up bounds for intermediate stages
    // u
    int idxbu[NBU];
    {% for i in range(end=ocp.dims.nbu) %}
    idxbu[{{i}}] = {{ocp.constraints.idxbu[i]}};
    {%- endfor %}
    double lbu[NBU]; 
    double ubu[NBU];
    {% for i in range(end=ocp.dims.nbu) %}
    lbu[{{i}}] = {{ ocp.constraints.lbu[i] }};
    ubu[{{i}}] = {{ ocp.constraints.ubu[i] }};
    {%- endfor %}
    
    // x
    int idxbx[NBX];
    {% for i in range(end=ocp.dims.nbx) %}
    idxbx[{{i}}] = {{ocp.constraints.idxbx[i]}};
    {%- endfor %}
    double lbx[NBX]; 
    double ubx[NBX];
    {% for i in range(end=ocp.dims.nbx) %}
    lbx[{{i}}] = {{ ocp.constraints.lbx[i] }};
    ubx[{{i}}] = {{ ocp.constraints.ubx[i] }};
    {%- endfor %}

    // set up general constraints for stage 0 to N-1 
    double D[NG*NU];
    double C[NG*NX];
    double lg[NG];
    double ug[NG];

    {% for j in range(end=ocp.dims.ng) %}
        {%- for k in range(end=ocp.dims.nx) %}
    D[{{j}}+NG * {{k}}] = {{ ocp.constraints.D[j][k] }}; 
        {%- endfor %}
    {%- endfor %}

    {% for j in range(end=ocp.dims.ng) %}
        {%- for k in range(end=ocp.dims.nu) %}
    C[{{j}}+NG * {{k}}] = {{ ocp.constraints.C[j][k] }}; 
        {%- endfor %}
    {%- endfor %}

    {% for i in range(end=ocp.dims.ng) %}
    lg[{{i}}] = {{ ocp.constraints.lg[i] }};
    {%- endfor %}

    {% for i in range(end=ocp.dims.ng) %}
    ug[{{i}}] = {{ ocp.constraints.ug[i] }};
    {%- endfor %}

    // set up nonlinear constraints for stage 0 to N-1 
    double lh[NH];
    double uh[NH];

    {% for i in range(end=ocp.dims.nh) %}
    lh[{{i}}] = {{ ocp.constraints.lh[i] }};
    {%- endfor %}

    {% for i in range(end=ocp.dims.nh) %}
    uh[{{i}}] = {{ ocp.constraints.uh[i] }};
    {%- endfor %}
    
    // set up bounds for last stage
    // x
    int idxbxN[NBXN];
    {% for i in range(end=ocp.dims.nbxN) %}
    idxbxN[{{i}}] = {{ocp.constraints.idxbxN[i]}};
    {%- endfor %}
    double lbxN[NBXN]; 
    double ubxN[NBXN];
    {% for i in range(end=ocp.dims.nbxN) %}
    lbxN[{{i}}] = {{ ocp.constraints.lbxN[i] }};
    ubxN[{{i}}] = {{ ocp.constraints.ubxN[i] }};
    {%- endfor %}
    
    // set up general constraints for last stage 
    double CN[NGN*NX];
    double lgN[NGN];
    double ugN[NGN];

    {% for j in range(end=ocp.dims.ngN) %}
        {%- for k in range(end=ocp.dims.nu) %}
    CN[{{j}}+NG * {{k}}] = {{ ocp.constraints.CN[j][k] }}; 
        {%- endfor %}
    {%- endfor %}

    {% for i in range(end=ocp.dims.ngN) %}
    lgN[{{i}}] = {{ ocp.constraints.lgN[i] }};
    {%- endfor %}

    {% for i in range(end=ocp.dims.ngN) %}
    ugN[{{i}}] = {{ ocp.constraints.ugN[i] }};
    {%- endfor %}

    // set up nonlinear constraints for last stage 
    double lhN[NHN];
    double uhN[NHN];

    {% for i in range(end=ocp.dims.nhN) %}
    lhN[{{i}}] = {{ ocp.constraints.lhN[i] }};
    {%- endfor %}

    {% for i in range(end=ocp.dims.nhN) %}
    uhN[{{i}}] = {{ ocp.constraints.uhN[i] }};
    {%- endfor %}

    double yref[NY];
    double W[NY*NY];

    double Vx[NY*NX];
    double Vu[NY*NU];
    double Vz[NY*NZ];

    double yrefN[NYN];
    double WN[NYN*NYN];

    double VxN[NYN*NX];
    
    for (int ii = 0; ii < NU + NX; ii++)
        yref[ii] = 0.0;

    {% for j in range(end=ocp.dims.ny) %}
        {%- for k in range(end=ocp.dims.ny) %}
    W[{{j}}+(NY) * {{k}}] = {{ ocp.cost.W[j][k] }}; 
        {%- endfor %}
    {%- endfor %}

    {% for j in range(end=ocp.dims.ny) %}
        {%- for k in range(end=ocp.dims.nx) %}
    Vx[{{j}}+(NY) * {{k}}] = {{ ocp.cost.Vx[j][k] }}; 
        {%- endfor %}
    {%- endfor %}

    {% for j in range(end=ocp.dims.ny) %}
        {%- for k in range(end=ocp.dims.nu) %}
    Vu[{{j}}+(NY) * {{k}}] = {{ ocp.cost.Vu[j][k] }}; 
        {%- endfor %}
    {%- endfor %}

    {% for j in range(end=ocp.dims.ny) %}
        {%- for k in range(end=ocp.dims.nz) %}
    Vz[{{j}}+(NY) * {{k}}] = {{ ocp.cost.Vz[j][k] }}; 
        {%- endfor %}
    {%- endfor %}

    {% for j in range(end=ocp.dims.ny) %}
    yref[{{j}}] = {{ ocp.cost.yref[j] }}; 
    {%- endfor %}

    {% for j in range(end=ocp.dims.nyN) %}
        {%- for k in range(end=ocp.dims.nyN) %}
    WN[{{j}}+(NYN) * {{k}}] = {{ ocp.cost.WN[j][k] }}; 
        {%- endfor %}
    {%- endfor %}

    {% for j in range(end=ocp.dims.nyN) %}
        {%- for k in range(end=ocp.dims.nx) %}
    VxN[{{j}}+(NYN) * {{k}}] = {{ ocp.cost.VxN[j][k] }}; 
        {%- endfor %}
    {%- endfor %}

    {% for j in range(end=ocp.dims.nyN) %}
    yrefN[{{j}}] = {{ ocp.cost.yrefN[j] }}; 
    {%- endfor %}

    int max_num_sqp_iterations = 100;

    int nx[N+1];
    int nu[N+1];
    int nbx[N+1];
    int nbu[N+1];
    int nb[N+1];
    int ng[N+1];
    int nh[N+1];
    int ns[N+1];
    int nz[N+1];
    int nv[N+1];
    int ny[N+1];
    int npd[N+1];
    int npdN[N+1];

    for(int i = 0; i < N+1; i++) {
        nx[i]  = NX_;
        nu[i]  = NU_;
        nbx[i] = NBX_;
        nbu[i] = NBU_;
        nb[i]  = NBU_ + NBX_;
        ng[i]  = NG_;
        nh[i]  = NH_;
        npd[i] = NPD_;
        ns[i]  = 0;
        nz[i]  = NZ_;
        nv[i]  = NX_ + NU_;
        ny[i]  = NY_;
    }

    nbx[0] = NX_;
    nbu[0] = NBU_;
    nb[0]  = NX_ + NBU_;

    nu[N]  = 0;
    nx[N]  = NX_;
    nz[N]  = 0;
    nh[N]  = NHN_;
    npd[N]  = NPDN_;
    nv[N]  = NX_; 
    ny[N]  = NYN_;
    nbu[N] = 0;
    nbx[N] = NBXN_;
    ng[N]  = NGN_;
    nb[N]  = NBXN_;

    // Make plan
    nlp_solver_plan = ocp_nlp_plan_create(N);
    {% if ocp.solver_config.nlp_solver_type == "SQP" %}
    nlp_solver_plan->nlp_solver = SQP;
    {% else %}
    nlp_solver_plan->nlp_solver = SQP_RTI;
    {% endif %}
    nlp_solver_plan->ocp_qp_solver_plan.qp_solver = {{ ocp.solver_config.qp_solver }};
    for (int i = 0; i <= N; i++)
        nlp_solver_plan->nlp_cost[i] = LINEAR_LS;
    for (int i = 0; i < N; i++)
    {
        nlp_solver_plan->nlp_dynamics[i] = CONTINUOUS_MODEL;
        nlp_solver_plan->sim_solver_plan[i].sim_solver = {{ ocp.solver_config.integrator_type}};
    }

    for (int i = 0; i < N; i++) {
        {% if ocp.dims.npd > 0 %}
        nlp_solver_plan->nlp_constraints[i] = BGHP;
        {% else %}
        nlp_solver_plan->nlp_constraints[i] = BGH;
        {% endif %}
    }

    {% if ocp.dims.npdN > 0 %}
    nlp_solver_plan->nlp_constraints[N] = BGHP;
    {% else %}
    nlp_solver_plan->nlp_constraints[N] = BGH;
    {% endif %}

    {% if ocp.solver_config.hessian_approx == "EXACT" %} 
    nlp_solver_plan->regularization = CONVEXIFICATION;
    {% endif %}
    nlp_config = ocp_nlp_config_create(*nlp_solver_plan);

    /* create and set ocp_nlp_dims */
	nlp_dims = ocp_nlp_dims_create(nlp_config);

    ocp_nlp_dims_set_opt_vars(nlp_config, nlp_dims, "nx", nx);
    ocp_nlp_dims_set_opt_vars(nlp_config, nlp_dims, "nu", nu);
    ocp_nlp_dims_set_opt_vars(nlp_config, nlp_dims, "nz", nz);
    ocp_nlp_dims_set_opt_vars(nlp_config, nlp_dims, "ns", ns);

    for (int i = 0; i <= N; i++) {
        ocp_nlp_dims_set_cost(nlp_config, nlp_dims, i, "ny", &ny[i]);
        ocp_nlp_dims_set_constraints(nlp_config, nlp_dims, i, "nbx", &nbx[i]);
        ocp_nlp_dims_set_constraints(nlp_config, nlp_dims, i, "nbu", &nbu[i]);
        ocp_nlp_dims_set_constraints(nlp_config, nlp_dims, i, "ng", &ng[i]);
        ocp_nlp_dims_set_constraints(nlp_config, nlp_dims, i, "nh", &nh[i]);
    }

    {% if ocp.dims.npd > 0 %}
    for (int i = 0; i < N; i++) 
        ocp_nlp_dims_set_constraints(nlp_config, nlp_dims, i, "np", &npd[i]);
    {% endif %}
    {% if ocp.dims.npdN > 0 %}
    ocp_nlp_dims_set_constraints(nlp_config, nlp_dims, N, "np", &npd[N]);
    {% endif %}

    {% if ocp.dims.npd > 0 %}
    p_constraint = (external_function_casadi *) malloc(sizeof(external_function_casadi)*N);
    for (int i = 0; i < N; ++i) {
        // nonlinear part of convex-composite constraint
        p_constraint[i].casadi_fun = &{{ ocp.con_p_name }}_p_constraint;
        p_constraint[i].casadi_n_in = &{{ ocp.con_p_name }}_p_constraint_n_in;
        p_constraint[i].casadi_n_out = &{{ ocp.con_p_name }}_p_constraint_n_out;
        p_constraint[i].casadi_sparsity_in = &{{ ocp.con_p_name }}_p_constraint_sparsity_in;
        p_constraint[i].casadi_sparsity_out = &{{ ocp.con_p_name }}_p_constraint_sparsity_out;
        p_constraint[i].casadi_work = &{{ ocp.con_p_name }}_p_constraint_work;

        external_function_casadi_create(&p_constraint[i]);
    }
    {% endif %}

    {% if ocp.dims.npdN > 0 %}
	// nonlinear part of convex-composite constraint
	external_function_casadi p_constraint_N;
	p_constraint_N.casadi_fun = &{{ ocp.con_pN_name }}_p_constraint_N;
	p_constraint_N.casadi_n_in = &{{ ocp.con_pN_name }}_p_constraint_N_n_in;
	p_constraint_N.casadi_n_out = &{{ ocp.con_pN_name }}_p_constraint_N_n_out;
	p_constraint_N.casadi_sparsity_in = &{{ ocp.con_pN_name }}_p_constraint_N_sparsity_in;
	p_constraint_N.casadi_sparsity_out = &{{ ocp.con_pN_name }}_p_constraint_N_sparsity_out;
	p_constraint_N.casadi_work = &{{ ocp.con_pN_name }}_p_constraint_N_work;

    external_function_casadi_create(p_constraint_N);
    {% endif %}

    {% if ocp.dims.nh > 0 %}
    h_constraint = (external_function_casadi *) malloc(sizeof(external_function_casadi)*N);
    for (int i = 0; i < N; ++i) {
        // nonlinear constraint
        h_constraint[i].casadi_fun = &{{ ocp.con_h_name }}_h_constraint;
        h_constraint[i].casadi_n_in = &{{ ocp.con_h_name }}_h_constraint_n_in;
        h_constraint[i].casadi_n_out = &{{ ocp.con_h_name }}_h_constraint_n_out;
        h_constraint[i].casadi_sparsity_in = &{{ ocp.con_h_name }}_h_constraint_sparsity_in;
        h_constraint[i].casadi_sparsity_out = &{{ ocp.con_h_name }}_h_constraint_sparsity_out;
        h_constraint[i].casadi_work = &{{ ocp.con_h_name }}_h_constraint_work;

        external_function_casadi_create(&h_constraint[i]);
    }
    {% endif %}

    {% if ocp.dims.nhN > 0 %}
	// nonlinear constraint
	external_function_casadi h_constraint_N;
	h_constraint_N.casadi_fun = &{{ ocp.con_hN_name }}_h_constraint_N;
	h_constraint_N.casadi_n_in = &{{ ocp.con_hN_name }}_h_constraint_N_n_in;
	h_constraint_N.casadi_n_out = &{{ ocp.con_hN_name }}_h_constraint_N_n_out;
	h_constraint_N.casadi_sparsity_in = &{{ ocp.con_hN_name }}_h_constraint_N_sparsity_in;
	h_constraint_N.casadi_sparsity_out = &{{ ocp.con_hN_name }}_h_constraint_N_sparsity_out;
	p_constraint_N.casadi_work = &{{ ocp.con_hN_name }}_h_constraint_N_work;

    external_function_casadi_create(h_constraint_N);
    {% endif %}

    {% if ocp.solver_config.integrator_type == "ERK" %}
    // explicit ode
    {% if ocp.dims.np < 1 %}
    forw_vde_casadi = (external_function_casadi *) malloc(sizeof(external_function_casadi)*N);
    {% else %}
    forw_vde_casadi = (external_function_param_casadi *) malloc(sizeof(external_function_param_casadi)*N);
    {% endif %}

    for (int i = 0; i < N; ++i) {
        forw_vde_casadi[i].casadi_fun = &{{ ocp.model_name }}_expl_vde_forw;
        forw_vde_casadi[i].casadi_n_in = &{{ ocp.model_name }}_expl_vde_forw_n_in;
        forw_vde_casadi[i].casadi_n_out = &{{ ocp.model_name }}_expl_vde_forw_n_out;
        forw_vde_casadi[i].casadi_sparsity_in = &{{ ocp.model_name }}_expl_vde_forw_sparsity_in;
        forw_vde_casadi[i].casadi_sparsity_out = &{{ ocp.model_name }}_expl_vde_forw_sparsity_out;
        forw_vde_casadi[i].casadi_work = &{{ ocp.model_name }}_expl_vde_forw_work;
        external_function_casadi_create(&forw_vde_casadi[i]);
    }

    {% if ocp.solver_config.hessian_approx == "EXACT" %} 
    external_function_casadi * hess_vde_casadi;
    {% if ocp.dims.np < 1 %}
    hess_vde_casadi = (external_function_casadi *) malloc(sizeof(external_function_casadi)*N);
    {% else %}
    hess_vde_casadi = (external_function_param_casadi *) malloc(sizeof(external_function_param_casadi)*N);
    {% endif %}
    for (int i = 0; i < N; ++i) {
        hess_vde_casadi[i].casadi_fun = &{{ ocp.model_name }}_expl_ode_hess;
        hess_vde_casadi[i].casadi_n_in = &{{ ocp.model_name }}_expl_ode_hess_n_in;
        hess_vde_casadi[i].casadi_n_out = &{{ ocp.model_name }}_expl_ode_hess_n_out;
        hess_vde_casadi[i].casadi_sparsity_in = &{{ ocp.model_name }}_expl_ode_hess_sparsity_in;
        hess_vde_casadi[i].casadi_sparsity_out = &{{ ocp.model_name }}_expl_ode_hess_sparsity_out;
        hess_vde_casadi[i].casadi_work = &{{ ocp.model_name }}_expl_ode_hess_work;
        external_function_casadi_create(&hess_vde_casadi[i]);
    }
    {% endif %}
    {% elif ocp.solver_config.integrator_type == "IRK" %}
    // implicit dae
    {% if ocp.dims.np < 1 %}
    impl_dae_fun = (external_function_casadi *) malloc(sizeof(external_function_casadi)*N);
    {% else %}
    impl_dae_fun = (external_function_param_casadi *) malloc(sizeof(external_function_param_casadi)*N);
    {% endif %}
    for (int i = 0; i < N; ++i) {
        impl_dae_fun[i].casadi_fun = &{{ ocp.model_name }}_impl_dae_fun;
        impl_dae_fun[i].casadi_work = &{{ ocp.model_name }}_impl_dae_fun_work;
        impl_dae_fun[i].casadi_sparsity_in = &{{ ocp.model_name }}_impl_dae_fun_sparsity_in;
        impl_dae_fun[i].casadi_sparsity_out = &{{ ocp.model_name }}_impl_dae_fun_sparsity_out;
        impl_dae_fun[i].casadi_n_in = &{{ ocp.model_name }}_impl_dae_fun_n_in;
        impl_dae_fun[i].casadi_n_out = &{{ ocp.model_name }}_impl_dae_fun_n_out;
        // TODO(fix this!!)
        {% if ocp.dims.np < 1 %}
        external_function_casadi_create(&impl_dae_fun[i]);
        {% else %}
        external_function_param_casadi_create(&impl_dae_fun[i], {{ocp.dims.np}});
        {% endif %}
    }

    {% if ocp.dims.np < 1 %}
    impl_dae_fun_jac_x_xdot_z = (external_function_casadi *) malloc(sizeof(external_function_casadi)*N);
    {% else %}
    impl_dae_fun_jac_x_xdot_z = (external_function_param_casadi *) malloc(sizeof(external_function_param_casadi)*N);
    {% endif %}
    for (int i = 0; i < N; ++i) {
        impl_dae_fun_jac_x_xdot_z[i].casadi_fun = &{{ ocp.model_name }}_impl_dae_fun_jac_x_xdot_z;
        impl_dae_fun_jac_x_xdot_z[i].casadi_work = &{{ ocp.model_name }}_impl_dae_fun_jac_x_xdot_z_work;
        impl_dae_fun_jac_x_xdot_z[i].casadi_sparsity_in = &{{ ocp.model_name }}_impl_dae_fun_jac_x_xdot_z_sparsity_in;
        impl_dae_fun_jac_x_xdot_z[i].casadi_sparsity_out = &{{ ocp.model_name }}_impl_dae_fun_jac_x_xdot_z_sparsity_out;
        impl_dae_fun_jac_x_xdot_z[i].casadi_n_in = &{{ ocp.model_name }}_impl_dae_fun_jac_x_xdot_z_n_in;
        impl_dae_fun_jac_x_xdot_z[i].casadi_n_out = &{{ ocp.model_name }}_impl_dae_fun_jac_x_xdot_z_n_out;
        {% if ocp.dims.np < 1 %}
        external_function_casadi_create(&impl_dae_fun_jac_x_xdot_z[i]);
        {% else %}
        external_function_param_casadi_create(&impl_dae_fun_jac_x_xdot_z[i], {{ocp.dims.np}});
        {% endif %}
    }

    {% if ocp.dims.np < 1 %}
    impl_dae_jac_x_xdot_u_z = (external_function_casadi *) malloc(sizeof(external_function_casadi)*N);
    {% else %}
    impl_dae_jac_x_xdot_u_z = (external_function_param_casadi *) malloc(sizeof(external_function_param_casadi)*N);
    {% endif %}
    for (int i = 0; i < N; ++i) {
        impl_dae_jac_x_xdot_u_z[i].casadi_fun = &{{ ocp.model_name }}_impl_dae_jac_x_xdot_u_z;
        impl_dae_jac_x_xdot_u_z[i].casadi_work = &{{ ocp.model_name }}_impl_dae_jac_x_xdot_u_z_work;
        impl_dae_jac_x_xdot_u_z[i].casadi_sparsity_in = &{{ ocp.model_name }}_impl_dae_jac_x_xdot_u_z_sparsity_in;
        impl_dae_jac_x_xdot_u_z[i].casadi_sparsity_out = &{{ ocp.model_name }}_impl_dae_jac_x_xdot_u_z_sparsity_out;
        impl_dae_jac_x_xdot_u_z[i].casadi_n_in = &{{ ocp.model_name }}_impl_dae_jac_x_xdot_u_z_n_in;
        impl_dae_jac_x_xdot_u_z[i].casadi_n_out = &{{ ocp.model_name }}_impl_dae_jac_x_xdot_u_z_n_out;
        {% if ocp.dims.np < 1 %}
        external_function_casadi_create(&impl_dae_jac_x_xdot_u_z[i]);
        {% else %}
        external_function_param_casadi_create(&impl_dae_jac_x_xdot_u_z[i], {{ocp.dims.np}});
        {% endif %}
    }
    {% endif %}

    nlp_in = ocp_nlp_in_create(nlp_config, nlp_dims);

    for (int i = 0; i < N; ++i)
        nlp_in->Ts[i] = Tf/N;

    // NLP cost linear least squares
    // C  // TODO(oj) this can be done using
    // // ocp_nlp_cost_set_model(nlp_config, nlp_dims, nlp_in, i, "Cyt", Cyt);
    // ocp_nlp_cost_ls_model **cost_ls = (ocp_nlp_cost_ls_model **) nlp_in->cost;
    // for (int i = 0; i <= N; ++i) {
    //     blasfeo_dgese(nv[i], ny[i], 0.0, &cost_ls[i]->Cyt, 0, 0);
    //     for (int j = 0; j < nu[i]; j++)
    //         BLASFEO_DMATEL(&cost_ls[i]->Cyt, j, nx[i]+j) = 1.0;
    //     for (int j = 0; j < nx[i]; j++)
    //         BLASFEO_DMATEL(&cost_ls[i]->Cyt, nu[i]+j, j) = 1.0;
    // }
    // W
    for (int i = 0; i < N; ++i) {
        ocp_nlp_cost_model_set(nlp_config, nlp_dims, nlp_in, i, "W", W);
    }
    // WN
    ocp_nlp_cost_model_set(nlp_config, nlp_dims, nlp_in, N, "W", WN);


	for (int i = 0; i < N; ++i) {
        ocp_nlp_cost_model_set(nlp_config, nlp_dims, nlp_in, i, "Vx", Vx);
        ocp_nlp_cost_model_set(nlp_config, nlp_dims, nlp_in, i, "Vu", Vu);
        ocp_nlp_cost_model_set(nlp_config, nlp_dims, nlp_in, i, "Vz", Vz);
        ocp_nlp_cost_model_set(nlp_config, nlp_dims, nlp_in, i, "yref", yref);
	}

    ocp_nlp_cost_model_set(nlp_config, nlp_dims, nlp_in, N, "Vx", VxN);
    ocp_nlp_cost_model_set(nlp_config, nlp_dims, nlp_in, N, "yref", yrefN);

    // NLP dynamics
    int set_fun_status;
    for (int i = 0; i < N; ++i) {
    {% if ocp.solver_config.integrator_type == "ERK" %} 
        set_fun_status = ocp_nlp_dynamics_model_set(nlp_config, nlp_dims, nlp_in, i, "expl_vde_for", &forw_vde_casadi[i]);
        if (set_fun_status != 0) { printf("Error while setting expl_vde_for[%i]\n", i);  exit(1); }
        {% if ocp.solver_config.hessian_approx == "EXACT" %} 
            set_fun_status = ocp_nlp_dynamics_model_set(nlp_config, nlp_dims, nlp_in, i, "expl_ode_hes", &hess_vde_casadi[i]);
            if (set_fun_status != 0) { printf("Error while setting expl_ode_hes[%i]\n", i);  exit(1); }
        {% endif %}
    {% elif ocp.solver_config.integrator_type == "IRK" %} 
        set_fun_status = ocp_nlp_dynamics_model_set(nlp_config, nlp_dims, nlp_in, i, "impl_ode_fun", &impl_dae_fun[i]);
        if (set_fun_status != 0) { printf("Error while setting impl_dae_fun[%i]\n", i);  exit(1); }
        set_fun_status = ocp_nlp_dynamics_model_set(nlp_config, nlp_dims, nlp_in, i, "impl_ode_fun_jac_x_xdot", &impl_dae_fun_jac_x_xdot_z[i]);
        if (set_fun_status != 0) { printf("Error while setting impl_dae_fun_jac_x_xdot_z[%i]\n", i);  exit(1); }
        set_fun_status = ocp_nlp_dynamics_model_set(nlp_config, nlp_dims, nlp_in, i, "impl_ode_jac_x_xdot_u", &impl_dae_jac_x_xdot_u_z[i]);
        if (set_fun_status != 0) { printf("Error while setting impl_dae_jac_x_xdot_u_z[%i]\n", i);  exit(1); }
    {% endif %}
    }

    // NLP constraints
    // TODO(oj) remove this when idxb setter available

    // bounds for stage 0
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, 0, "idxbx", idxbx0);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, 0, "lbx", lbx0);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, 0, "ubx", ubx0);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, 0, "idxbu", idxbu0);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, 0, "lbu", lbu0);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, 0, "ubu", ubu0);

    // bounds for intermediate stages
    for (int i = 1; i < N; ++i)
    {
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "idxbx", idxbx);
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "lbx", lbx);
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "ubx", ubx);
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "idxbu", idxbu);
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "lbu", lbu);
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "ubu", ubu);
    }
   
    {% if ocp.dims.ng > 0 %} 
    // general constraints for stages 0 to N-1
    for (int i = 0; i < N; ++i)
    {
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "D", D);
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "C", C);
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "lg", lg);
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "ug", ug);
    }
    {% endif %}
    
    {% if ocp.dims.nbxN > 0 %} 
    // bounds for last
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, N, "idxbx", idxbxN);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, N, "lbx", lbxN);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, N, "ubx", ubxN);
    {% endif %}
    
    {% if ocp.dims.ngN > 0 %} 
    // general constraints for last stage
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, N, "C", CN);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, N, "lg", lgN);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, N, "ug", ugN);
    {% endif %}

    
    {% if ocp.dims.npd > 0 %}
    // convex-composite constraints for stages 0 to N-1
    for (int i = 0; i < N; ++i)
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "p", &p_constraint[i]);
    {% endif %}

    {% if ocp.dims.npdN > 0 %}
    // convex-composite constraints for stage N
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, N, "p", &p_constraint_N[i]);
    {% endif %}

    {% if ocp.dims.nh > 0 %}
    // nonlinear constraints for stages 0 to N-1
    for (int i = 0; i < N; ++i)
    {
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "nl_constr_h_fun_jac", &h_constraint[i]);
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "lh", lh);
        ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, i, "uh", uh);
    }
    {% endif %}

    {% if ocp.dims.nhN > 0 %}
    // nonlinear constraints for stage N
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, N, "nl_constr_h_fun_jac", &h_constraint_N[i]);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, N, "lh", lhN);
    ocp_nlp_constraints_model_set(nlp_config, nlp_dims, nlp_in, N, "uh", uhN);
    {% endif %}

    nlp_opts = ocp_nlp_opts_create(nlp_config, nlp_dims);
    
    {% if ocp.dims.nz > 0 %}
    bool output_z_val = true; 
    bool sens_algebraic_val = true; 
    int num_steps_val = 1; 
    for (int i = 0; i < N; i++) ocp_nlp_dynamics_opts_set(nlp_config, nlp_opts, i, "output_z", &output_z_val);
    for (int i = 0; i < N; i++) ocp_nlp_dynamics_opts_set(nlp_config, nlp_opts, i, "sens_algebraic", &sens_algebraic_val);
    for (int i = 0; i < N; i++) ocp_nlp_dynamics_opts_set(nlp_config, nlp_opts, i, "num_steps", &num_steps_val);
    {% endif %}
    int ns_val = 1; 
    for (int i = 0; i < N; i++) ocp_nlp_dynamics_opts_set(nlp_config, nlp_opts, i, "ns", &ns_val);
    bool jac_reuse_val = true;
    for (int i = 0; i < N; i++) ocp_nlp_dynamics_opts_set(nlp_config, nlp_opts, i, "jac_reuse", &jac_reuse_val);

    {% if ocp.solver_config.nlp_solver_type == "SQP" %}

    int maxIter = max_num_sqp_iterations;
    double min_res_g = 1e-6;
    double min_res_b = 1e-6;
    double min_res_d = 1e-6;
    double min_res_m = 1e-6;

    ocp_nlp_opts_set(nlp_config, nlp_opts, "maxIter", &maxIter);
    ocp_nlp_opts_set(nlp_config, nlp_opts, "min_res_g", &min_res_g);
    ocp_nlp_opts_set(nlp_config, nlp_opts, "min_res_b", &min_res_b);
    ocp_nlp_opts_set(nlp_config, nlp_opts, "min_res_d", &min_res_d);
    ocp_nlp_opts_set(nlp_config, nlp_opts, "min_res_m", &min_res_m);


    {% else %}
    // ocp_nlp_sqp_rti_opts *sqp_opts = (ocp_nlp_sqp_rti_opts *) nlp_opts;
    {% endif %}
    {% if ocp.solver_config.hessian_approx == "EXACT" %}
    for (int i = 0; i < N; ++i)
    {
        // TODO(oj) is the following needed, and what does it do? do we
        // ((ocp_nlp_dynamics_cont_opts *) sqp_opts->dynamics[i])->compute_hess = true;
        int num_steps = 5;
        bool sens_hess = true;
        bool sens_adj = true;

        ocp_nlp_dynamics_opts_set(nlp_config, nlp_opts, i, "num_steps", &num_steps);
        ocp_nlp_dynamics_opts_set(nlp_config, nlp_opts, i, "sens_hess", &sens_hess);
        ocp_nlp_dynamics_opts_set(nlp_config, nlp_opts, i, "sens_adj", &sens_adj);
    }
    {% endif %}

    nlp_out = ocp_nlp_out_create(nlp_config, nlp_dims);
    for (int i = 0; i <= N; ++i) {
        blasfeo_dvecse(nu[i]+nx[i], 0.0, nlp_out->ux+i, 0);
    }
    
    nlp_solver = ocp_nlp_solver_create(nlp_config, nlp_dims, nlp_opts);

    // initialize parameters to nominal value
    {% if ocp.dims.np > 0%}
    double p[{{ocp.dims.np}}];
    {% for i in range(end=ocp.dims.np) %}
    p[{{i}}] = {{ocp.constraints.p[i]}};
    {%- endfor %}
    {% if ocp.solver_config.integrator_type == "IRK" %}
    for (int ii = 0; ii < {{ocp.dims.N}}; ii++) {
    impl_dae_fun[ii].set_param(impl_dae_fun+ii, p);
    impl_dae_fun_jac_x_xdot_z[ii].set_param(impl_dae_fun_jac_x_xdot_z+ii, p);
    impl_dae_jac_x_xdot_u_z[ii].set_param(impl_dae_jac_x_xdot_u_z+ii, p);
    }
    {% else %}
    for (int ii = 0; ii < {{ocp.dims.N}}; ii++) {
    expl_vde_for[ii].set_param(expl_vde_for+ii, p);
    }
    {% endif %}
    {% endif %}

    return status;
}

int acados_solve() {

    // solve NLP 
    int solver_status = ocp_nlp_solve(nlp_solver, nlp_in, nlp_out);

    return solver_status;
}

int acados_free() {

    // free memory
    ocp_nlp_opts_destroy(nlp_opts);
    ocp_nlp_in_destroy(nlp_in);
    ocp_nlp_out_destroy(nlp_out);
    ocp_nlp_solver_destroy(nlp_solver);
    ocp_nlp_dims_destroy(nlp_dims);
    ocp_nlp_config_destroy(nlp_config);
    ocp_nlp_plan_destroy(nlp_solver_plan);

    // free external function 
    {% if ocp.solver_config.integrator_type == "IRK" %}
    for(int i = 0; i < {{ocp.dims.N}}; i++) {
        {% if ocp.dims.np < 1 %}
        external_function_casadi_free(&impl_dae_fun[i]);
        external_function_casadi_free(&impl_dae_fun_jac_x_xdot_z[i]);
        external_function_casadi_free(&impl_dae_jac_x_xdot_u_z[i]);
        {% else %}
        external_function_param_casadi_free(&impl_dae_fun[i]);
        external_function_param_casadi_free(&impl_dae_fun_jac_x_xdot_z[i]);
        external_function_param_casadi_free(&impl_dae_jac_x_xdot_u_z[i]);
        {% endif %}
    }
    {% else %}
    for(int i = 0; i < {{ocp.dims.N}}; i++) {
        {% if ocp.dims.np < 1 %}
        external_function_casadi_free(&forw_vde_casadi[i]);
        {% else %}
        external_function_param_casadi_free(&forw_vde_casadi[i]);
        {% endif %}
    {% if ocp.solver_config.hessian_approx == "EXACT" %}
        {% if ocp.dims.np < 1 %}
        external_function_casadi_free(&hess_vde_casadi[i]);
        {% else %}
        external_function_param_casadi_free(&hess_vde_casadi[i]);
        {% endif %}
    {% endif %}
    }
    {% endif %}
    
    return 0;
}

ocp_nlp_in * acados_get_nlp_in() { return  nlp_in; }
ocp_nlp_out * acados_get_nlp_out() { return  nlp_out; }
ocp_nlp_solver * acados_get_nlp_solver() { return  nlp_solver; }
ocp_nlp_config * acados_get_nlp_config() { return  nlp_config; }
void * acados_get_nlp_opts() { return  nlp_opts; }
ocp_nlp_dims * acados_get_nlp_dims() { return  nlp_dims; }