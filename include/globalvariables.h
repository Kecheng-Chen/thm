#pragma once

#include <iostream>
#include <string>

#include "csv.h"
#include <vector>

// std::vector<double> coord_perm(11,0);
// std::vector<double> perm_value(11,0);

// void get_parameter(std::string filename, std::vector<double>&
// coord,std::vector<double>& data);

// get_parameter("parameters.txt", coord_perm, perm_value);//1e-5/1000/9.8;

namespace EquationData {

double g_perm = 1e-7 / 1000 / 9.8;  // permeability
const double g_c_T = 1.2e6;         // heat capacity of the mixture
const double g_lam = 1.2;           // heat conductivity
const double g_c_w = 1e6;           // heat capacity of water
const double g_B_w = 1e5;           // bulk modulus of pores

// std::vector<std::vector<double>> g_perm_list = {
//     {0, 0}};  // permeablity list used for interpolation

// Pressure seetings
// const int g_num_P_bnd_id = 1;  // numbers of  pressure boudnary condition id
// const int g_P_bnd_id[g_num_P_bnd_id] = {5};  // pressure boundary condition
// id const double g_Pb_top = 100000;              // pressure at the top of
// model const double g_P_grad = 1000 * 9.8;  // pressure gradient in vertial
// direction
const int g_num_P_bnd_id = 1;  // numbers of  pressure boudnary condition id
const int g_P_bnd_id[g_num_P_bnd_id] = {6};  // pressure boundary condition id
const double g_Pb_top = 10000;                    // pressure at the top of model
const double g_P_grad = 0;  // pressure gradient in vertial direction

// Velocity settings
// const int g_num_QP_bnd_id = 1;  // numbers of velocity boudnary condition id
// const int g_QP_bnd_id[g_num_QP_bnd_id] = {2};  // velocity  boundary
// condition
//                                                // id
// const double g_Qb_well = -0.001;               // wellbore temperature
const int g_num_QP_bnd_id = 1;  // numbers of velocity boudnary condition id
const int g_QP_bnd_id[g_num_QP_bnd_id] = {1};  // velocity  boundary condition
                                               // id
const double g_Qb_well = -0.000001;               // wellbore temperature

// Temperature seetings
// const int g_num_T_bnd_id = 4;  // numbers of  temperature boudnary condition
// id const int g_T_bnd_id[g_num_T_bnd_id] = {
//     2, 3, 4, 5};                      //  temperature boudnary condition id
// const double g_Tb_well = 273.15 + 5;  // wellbore temperature
// const double g_Tb_top = 273.15 + 5;   // termperature at the top of model
// const double g_T_grad = 0.05;  // temperature gradient in verital direction
const int g_num_T_bnd_id = 2;  // numbers of  temperature boudnary condition id
const int g_T_bnd_id[g_num_T_bnd_id] = {
   1, 6};                  //  temperature boudnary condition id
const double g_Tb_well = 280.15;     // wellbore temperature
const double g_Tb_top = 273.15 + 5;  // termperature at the top of model
const double g_T_grad = 0;  // temperature gradient in verital direction

// Heat flow rate settins
// const int g_num_QT_bnd_id = 1;  // numbers of velocity boudnary condition id
// const int g_QT_bnd_id[g_num_QT_bnd_id] = {2};  // velocity  boundary
// condition
//                                                // id
// const double g_QT_well = 0;                    // wellbore temperature
const int g_num_QT_bnd_id = 2;  // numbers of velocity boudnary condition id
const int g_QT_bnd_id[g_num_QT_bnd_id] = {3, 4};  // velocity  boundary
                                                  // condition id
const double g_QT_well = 0;                       // wellbore temperature
const double g_QT_top = -g_lam * g_T_grad;
const double g_QT_bottom = g_lam * g_T_grad;

// solver settings
// const bool is_linspace = true;
// const double g_period = 3600 * 24;  // simulation time
// const int g_n_time_step = 21;       // simulation time
// std::vector<double> g_time_sequence = {
//     0,   1,   2,  3,  34,
//     5,   6,   7,  14,  24};
// const char time_unit = 'h';
const bool is_linspace = false;
const double g_period = 10;  // simulation time
const int g_n_time_step = 10;         // simulation time
std::vector<double> g_time_sequence = {0,  1, 2, 3,4, 5, 6, 7, 8, 9, 10};
const char time_unit = 'd';
const unsigned int n_g_P_max_iteration = 1000;
const unsigned int n_g_T_max_iteration = 4000;
const double g_P_tol_residual = 1e-8;
const double g_T_tol_residual = 1e-8;

// //dimention of the input data file (parameters_for_interpolation.txt in inputfiles is used in the example)
// const int dimension=3;
// //point number in x, y and z directions
// std::string file_name_interpolation="inputfiles/parameters_for_interpolation.txt";
// const int dimension_x=2;
// const int dimension_y=2;
// const int dimension_z=7;

}  // namespace EquationData
