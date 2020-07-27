#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <vector>

#include <deal.II/base/conditional_ostream.h>
#include <deal.II/base/convergence_table.h>
#include <deal.II/base/function.h>
#include <deal.II/base/function_lib.h>
#include <deal.II/base/logstream.h>
#include <deal.II/base/mpi.h>
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/utilities.h>

#include <deal.II/lac/block_sparsity_pattern.h>
#include <deal.II/lac/constraint_matrix.h>
#include <deal.II/lac/dynamic_sparsity_pattern.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/solver_gmres.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparsity_tools.h>
#include <deal.II/lac/vector.h>

// shared tria:
#include <deal.II/distributed/shared_tria.h>
// MPI support):
#include <deal.II/lac/petsc_parallel_sparse_matrix.h>
#include <deal.II/lac/petsc_parallel_vector.h>

// for parallel computing
#include <deal.II/lac/petsc_precondition.h>
#include <deal.II/lac/petsc_solver.h>
#include <deal.II/lac/petsc_sparse_matrix.h>
#include <deal.II/lac/petsc_vector.h>

#include <deal.II/dofs/dof_renumbering.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/grid_out.h>
#include <deal.II/grid/grid_refinement.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>

#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_values.h>

#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/error_estimator.h>
#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/solution_transfer.h>
#include <deal.II/numerics/vector_tools.h>

#include "interpolation.h"

using namespace dealii;

template <int dim>
class CoupledTH {
 public:
  CoupledTH(const unsigned int degree);
  ~CoupledTH();
  void run();

 private:
  void make_grid();
  void setup_system();
  void assemble_T_system();
  void assemble_P_system();
  void linear_solve_P();
  void linear_solve_T();
  void output_results(PETScWrappers::MPI::Vector&, std::string) const;

  MPI_Comm mpi_communicator;
  const unsigned int n_mpi_processes;
  const unsigned int this_mpi_process;
  ConditionalOStream pcout;

  IndexSet locally_owned_dofs;
  IndexSet locally_relevant_dofs;


  Triangulation<dim> triangulation;  // grid
  DoFHandler<dim> dof_handler;       // grid<->eleemnt

  const unsigned int degree;  // element degree


  FE_Q<dim> fe;  // element type

  // ConstraintMatrix constraints;  // hanging node
  PETScWrappers::MPI::SparseMatrix P_system_matrix;  // M_P + K_P
  PETScWrappers::MPI::SparseMatrix T_system_matrix;  // M_T + K_T

  PETScWrappers::MPI::Vector P_solution;      // P solution at n
  PETScWrappers::MPI::Vector T_solution;      // T solution at n
  PETScWrappers::MPI::Vector old_P_solution;  // P solution at n-1
  PETScWrappers::MPI::Vector old_T_solution;  // T solution at n-1
  PETScWrappers::MPI::Vector P_system_rhs;    // right hand side of P
                                              // system
  PETScWrappers::MPI::Vector T_system_rhs;    // right hand side of T
                                              // system

  double time;                   // t
  unsigned int timestep_number;  // n_t
  std::vector<double> time_sequence;

  double period;
  int n_time_step;
  double time_step;

  unsigned int P_iteration_namber;
  unsigned int T_iteration_namber;
  unsigned int n_P_max_iteration = EquationData::n_g_P_max_iteration;
  unsigned int n_T_max_iteration = EquationData::n_g_T_max_iteration;
  double P_tol_residual = EquationData::g_P_tol_residual;
  double T_tol_residual = EquationData::g_T_tol_residual;

  //edit by Yuan 07/22/2020
  Interpolation<3> data_interpolation;
};

template <int dim>
CoupledTH<dim>::CoupledTH(const unsigned int degree) : // initialization
    //triangulation(MPI_COMM_WORLD), 
      mpi_communicator(MPI_COMM_WORLD),
      n_mpi_processes(Utilities::MPI::n_mpi_processes(mpi_communicator)),
      this_mpi_process(Utilities::MPI::this_mpi_process(mpi_communicator)),
      pcout(std::cout,
            (this_mpi_process == 0)),  // output the results at process 0
      dof_handler(triangulation),
      fe(degree),
      degree(degree),
      time(0.0),
      timestep_number(0),
      T_iteration_namber(0),
      P_iteration_namber(0),
      //edit by Yuan 07/22/2020
      data_interpolation(EquationData::dimension_x,EquationData::dimension_y,EquationData::dimension_z,EquationData::file_name_interpolation)
       {
  if (EquationData::is_linspace) {
    period = EquationData::g_period;
    n_time_step = EquationData::g_n_time_step;
    time_sequence = linspace(0.0, period, n_time_step);
    time_step = time_sequence[1] - time_sequence[0];
  } else {
    time_sequence = EquationData::g_time_sequence;
    n_time_step = time_sequence.size();
    period = time_sequence[n_time_step - 1];
    time_step = time_sequence[1] - time_sequence[0];
  }
}

template <int dim>
CoupledTH<dim>::~CoupledTH() {
  dof_handler.clear();
}

template <int dim>
void CoupledTH<dim>::make_grid() {


  GridIn<dim> gridin;  // instantiate a gridinput
  gridin.attach_triangulation(triangulation);
  std::ifstream f("inputfiles/mesh.msh");
  gridin.read_msh(f);
    // print_mesh_info(triangulation, "outputfiles/grid-1.eps");
}


template <int dim>
void CoupledTH<dim>::setup_system() {



  GridTools::partition_triangulation(n_mpi_processes,
                                     triangulation);  // partition triangulation

  dof_handler.distribute_dofs(fe);  // distribute dofs to grid globle
  // dof_handler.distribute_dofs(fe);  // distribute dofs to grid
  DoFRenumbering::subdomain_wise(dof_handler);  // mapping from globle to local

  pcout << "Number of active cells: " << triangulation.n_active_cells()
        << " (on " << triangulation.n_levels() << " levels)" << std::endl
        << "Number of degrees of freedom: " << 2 * dof_handler.n_dofs() << " ("
        << dof_handler.n_dofs() << '+' << dof_handler.n_dofs() << ')'
        << std::endl
        << std::endl;

  const std::vector<IndexSet> locally_owned_dofs_per_proc =
      DoFTools::locally_owned_dofs_per_subdomain(dof_handler);
  locally_owned_dofs =
      locally_owned_dofs_per_proc[this_mpi_process];

  // sparsity pattern
  DynamicSparsityPattern dsp(dof_handler.n_dofs());  // sparsity
  DoFTools::make_sparsity_pattern(dof_handler, dsp);

  // forming system matrixes and initialize these matrixesy
  T_system_matrix.reinit(locally_owned_dofs, locally_owned_dofs, dsp,
                         mpi_communicator);
  T_system_rhs.reinit(locally_owned_dofs, mpi_communicator);
  T_solution.reinit(locally_owned_dofs, mpi_communicator);
  old_T_solution.reinit(locally_owned_dofs, mpi_communicator);


  // forming system matrixes and initialize these matrixes
  P_system_matrix.reinit(locally_owned_dofs, locally_owned_dofs, dsp,
                         mpi_communicator);
  P_system_rhs.reinit(locally_owned_dofs, mpi_communicator);
  P_solution.reinit(locally_owned_dofs, mpi_communicator);
  old_P_solution.reinit(locally_owned_dofs, mpi_communicator);

}

template <int dim>
void CoupledTH<dim>::assemble_P_system() {
  cbgeo::Clock timer;
  timer.tick();

  // reset matreix to zero
  P_system_matrix = 0;
  P_system_rhs = 0;
  P_solution = 0;

  QGauss<dim> quadrature_formula(degree + 1);
  QGauss<dim - 1> face_quadrature_formula(degree + 1);

  // Getting T values
  FEValues<dim> fe_values(fe, quadrature_formula,
                            update_values | update_gradients |
                                update_quadrature_points | update_JxW_values);

  FEFaceValues<dim> fe_face_values(fe, face_quadrature_formula,
                                     update_values | update_normal_vectors |
                                         update_quadrature_points |
                                         update_JxW_values);

  const unsigned int dofs_per_cell = fe.dofs_per_cell;
  const unsigned int n_q_points = quadrature_formula.size();
  const unsigned int n_face_q_points = face_quadrature_formula.size();

  // store the value at previous step at q_point for T
  std::vector<double> old_T_sol_values(n_q_points);
  std::vector<Tensor<1, dim>> old_T_sol_grads(n_q_points);

  // store the value at previous step at q_point for P
  std::vector<double> old_P_sol_values(n_q_points);
  std::vector<Tensor<1, dim>> old_P_sol_grads(n_q_points);

  // store the rhs and bd and old solution value at q_point of element for P
  std::vector<double> P_source_values(n_q_points);
  std::vector<double> QP_bd_values(n_face_q_points);

  // store the coordinate of gauss point
  // const Point<dim> P_quadrature_coord;

  //  local element matrix
  FullMatrix<double> P_local_mass_matrix(dofs_per_cell, dofs_per_cell);
  FullMatrix<double> P_local_stiffness_matrix(dofs_per_cell, dofs_per_cell);
  Vector<double> P_local_rhs(dofs_per_cell);
  std::vector<types::global_dof_index> P_local_dof_indices(dofs_per_cell);

  // boudnary condition and source term
  EquationData::PressureSourceTerm<dim> P_source_term;
  EquationData::PressureNeumanBoundaryValues<dim> QP_boundary;
  EquationData::PressureDirichletBoundaryValues<dim> P_boundary;

  // loop for cell
  typename DoFHandler<dim>::active_cell_iterator cell =
                                                     dof_handler.begin_active(),
                                                 endc = dof_handler.end();
  for (; cell != endc; ++cell) {
    if (cell->subdomain_id() ==
        this_mpi_process) {  // only assemble the system on cells that acturally
                             // belong to this MPI process
      // initialization
      P_local_mass_matrix = 0;
      P_local_stiffness_matrix = 0;
      P_local_rhs = 0;
      fe_values.reinit(cell);

      // get the values at gauss point old_T_sol_values from the system
      // old_T_solution
      fe_values.get_function_values(old_T_solution, old_T_sol_values);
      fe_values.get_function_gradients(old_T_solution, old_T_sol_grads);
      fe_values.get_function_values(old_P_solution, old_P_sol_values);
      fe_values.get_function_gradients(old_P_solution, old_P_sol_grads);

      // get source term value at the gauss point
      P_source_term.set_time(time);
      P_source_term.value_list(fe_values.get_quadrature_points(),
                               P_source_values);  // 一列q个

      // loop for q_point ASSMBLING CELL METRIX (weak form equation writing)
      for (unsigned int q = 0; q < n_q_points; ++q) {

        const auto P_quadrature_coord = fe_values.quadrature_point(q);

        //edit by Yuan 07/22/2020 delete
        // EquationData::g_perm = interpolate1d(
        //     EquationData::g_perm_list, P_quadrature_coord[2], false);  // step-5

        //edit by Yuan 07/22/2020 add
         EquationData::g_perm = data_interpolation.value(P_quadrature_coord[0],P_quadrature_coord[1],P_quadrature_coord[2]);
        

        for (unsigned int i = 0; i < dofs_per_cell; ++i) {
          const Tensor<1, dim> grad_phi_i_P = fe_values.shape_grad(i, q);
          const double phi_i_P = fe_values.shape_value(i, q);
          for (unsigned int j = 0; j < dofs_per_cell; ++j) {
            const Tensor<1, dim> grad_phi_j_P = fe_values.shape_grad(j, q);
            const double phi_j_P = fe_values.shape_value(j, q);
            P_local_mass_matrix(i, j) +=
                (phi_i_P * phi_j_P * fe_values.JxW(q));
            P_local_stiffness_matrix(i, j) +=
                 (time_step * EquationData::g_perm * EquationData::g_B_w *
                  grad_phi_i_P * grad_phi_j_P * fe_values.JxW(q));

          }
             P_local_rhs(i) += (time_step * phi_i_P * P_source_values[q] +
                              time_step * grad_phi_i_P * (Point<dim>(0, 0, 1)) *
                                  (-EquationData::g_B_w * EquationData::g_perm *
                                   EquationData::g_P_grad) +
                              phi_i_P * old_P_sol_values[q]) *
                             fe_values.JxW(q);

        }
      }

      // APPLIED NEWMAN BOUNDARY CONDITION
      for (int bd_i = 0; bd_i < EquationData::g_num_QP_bnd_id; bd_i++) {
        for (unsigned int face_no = 0;
             face_no < GeometryInfo<dim>::faces_per_cell; ++face_no) {
          if (cell->at_boundary(face_no) &&
              cell->face(face_no)->boundary_id() ==
                  EquationData::g_QP_bnd_id[bd_i]) {
            fe_face_values.reinit(cell, face_no);

            // get boundary condition
            QP_boundary.set_time(time);
            QP_boundary.set_boundary_id(*(EquationData::g_QP_bnd_id + bd_i));
            QP_boundary.value_list(fe_face_values.get_quadrature_points(),
                                   QP_bd_values);

            for (unsigned int q = 0; q < n_face_q_points; ++q) {

              const auto P_face_quadrature_coord =
                  fe_face_values.quadrature_point(q);

               EquationData::g_perm = data_interpolation.value(P_face_quadrature_coord[0],P_face_quadrature_coord[1],P_face_quadrature_coord[2]);

              for (unsigned int i = 0; i < dofs_per_cell; ++i) {
                P_local_rhs(i) += -time_step * EquationData::g_B_w *
                                  (fe_face_values.shape_value(i, q) *
                                   QP_bd_values[q] * fe_face_values.JxW(q));
              }
            }
          }
        }
      }

      // local ->globe
      cell->get_dof_indices(P_local_dof_indices);

      for (unsigned int i = 0; i < dofs_per_cell; ++i) {
        for (unsigned int j = 0; j < dofs_per_cell; ++j) {
          P_system_matrix.add(P_local_dof_indices[i], P_local_dof_indices[j],
                              P_local_mass_matrix(i, j));  // sys_mass matrix
          P_system_matrix.add(
              P_local_dof_indices[i], P_local_dof_indices[j],
              P_local_stiffness_matrix(i, j));  // sys_stiff matrix
        }
        P_system_rhs(P_local_dof_indices[i]) += P_local_rhs(i);
      }
    }
  }

  // compress matrix his means that each process sends the additions that were
  // made to those entries of the matrix and vector that the process did not own
  // itself to the process that owns them.fter receiving these additions from
  // other processes, each process then adds them to the values it already has.
  // These additions are combining the integral contributions of shape functions
  // living on several cells just as in a serial computation, with the
  // difference that the cells are assigned to different processes.
  P_system_matrix.compress(VectorOperation::add);
  P_system_rhs.compress(VectorOperation::add);

  // // ADD DIRICLET BOUNDARY
  {

    for (int bd_i = 0; bd_i < EquationData::g_num_P_bnd_id; bd_i++) {

      P_boundary.set_time(time);
      P_boundary.set_boundary_id(*(EquationData::g_P_bnd_id + bd_i));
      std::map<types::global_dof_index, double> P_bd_values;
      VectorTools::interpolate_boundary_values(
          dof_handler, *(EquationData::g_P_bnd_id + bd_i), P_boundary,
          P_bd_values);  // i表示边界的index
      MatrixTools::apply_boundary_values(
          P_bd_values, P_system_matrix, P_solution, P_system_rhs,
          false);  // The reason why we may not want to make the matrix
                   // symmetric is because this would require us to write into
                   // column entries that actually reside on other processes,
                   // i.e., it involves communicating data. This is always
                   // expensive. Experience tells us that CG also works (and
                   // works almost as well) if we don't remove the columns
                   // associated with boundary nodes, which can be explained by
                   // the special structure of this particular non-symmetry.
    }
  }
  timer.tock("assemble_P_system");
}

template <int dim>
void CoupledTH<dim>::assemble_T_system() {
  cbgeo::Clock timer;
  timer.tick();
  // reset matreix to zero
  T_system_matrix = 0;
  T_system_rhs = 0;
  T_solution = 0;


  QGauss<dim> quadrature_formula(degree + 1);
  QGauss<dim - 1> face_quadrature_formula(degree + 1);

  // Getting values
  FEValues<dim> fe_values(fe, quadrature_formula,
                            update_values | update_gradients |
                                update_quadrature_points | update_JxW_values);

  FEFaceValues<dim> fe_face_values(fe, face_quadrature_formula,
                                     update_values | update_normal_vectors |
                                         update_quadrature_points |
                                         update_JxW_values);

  const unsigned int dofs_per_cell = fe.dofs_per_cell; 
  const unsigned int n_q_points = quadrature_formula.size();
  const unsigned int n_face_q_points = face_quadrature_formula.size();

  // store the value at previous step at q_point for T
  std::vector<double> old_T_sol_values(n_q_points);
  std::vector<Tensor<1, dim>> old_T_sol_grads(n_q_points);

  // store the value at previous step at q_point for P
  std::vector<double> old_P_sol_values(n_q_points);
  std::vector<Tensor<1, dim>> old_P_sol_grads(n_q_points);

  // store the source and bd value at q_point
  std::vector<double> T_source_values(n_q_points);
  std::vector<double> QT_bd_values(n_face_q_points);

  //  local element matrix
  FullMatrix<double> T_local_mass_matrix(dofs_per_cell, dofs_per_cell);
  FullMatrix<double> T_local_stiffness_matrix(dofs_per_cell, dofs_per_cell);
  FullMatrix<double> T_local_convection_matrix(dofs_per_cell,
                                               dofs_per_cell);
  Vector<double> T_local_rhs(dofs_per_cell);
  std::vector<types::global_dof_index> T_local_dof_indices(dofs_per_cell);

  // boudnary condition
  EquationData::TemperatureSourceTerm<dim> T_source_term;
  EquationData::TemperatureNeumanBoundaryValues<dim> QT_boundary;
  EquationData::TemperatureDirichletBoundaryValues<dim> T_boundary;

  // loop for cell
  typename DoFHandler<dim>::active_cell_iterator cell =
                                                     dof_handler.begin_active(),
                                                 endc = dof_handler.end();
  for (; cell != endc; ++cell) {
    if (cell->subdomain_id() == this_mpi_process) {
      // initialization
      T_local_mass_matrix = 0;
      T_local_stiffness_matrix = 0;
      T_local_convection_matrix = 0;
      T_local_rhs = 0;
      fe_values.reinit(cell);
      // get the values at gauss point
      fe_values.get_function_values(old_T_solution, old_T_sol_values);
      fe_values.get_function_gradients(old_T_solution, old_T_sol_grads);
      fe_values.get_function_values(old_P_solution, old_P_sol_values);
      fe_values.get_function_gradients(old_P_solution, old_P_sol_grads);
      // get right hand side
      T_source_term.set_time(time);
      T_source_term.value_list(fe_values.get_quadrature_points(),
                               T_source_values);

      // loop for q_point ASSMBLING CELL METRIX (weak form equation writing)
      for (unsigned int q = 0; q < n_q_points; ++q) {
        const auto T_quadrature_coord = fe_values.quadrature_point(q);
        //edit by Yuan 07/22/2020 delete
        // EquationData::g_perm = interpolate1d(
        //     EquationData::g_perm_list, T_quadrature_coord[2], false);  // step-5
        //edit by Yuan 07/22/2020 add
        EquationData::g_perm = data_interpolation.value(T_quadrature_coord[0], T_quadrature_coord[1], T_quadrature_coord[2]);
        for (unsigned int i = 0; i < dofs_per_cell; ++i) {
          const Tensor<1, dim> grad_phi_i_T = fe_values.shape_grad(i, q);
          const double phi_i_T = fe_values.shape_value(i, q);
          for (unsigned int j = 0; j < dofs_per_cell; ++j) {
            const Tensor<1, dim> grad_phi_j_T = fe_values.shape_grad(j, q);
            const double phi_j_T = fe_values.shape_value(j, q);
            T_local_mass_matrix(i, j) +=
                (phi_i_T * phi_j_T * fe_values.JxW(q));
            T_local_stiffness_matrix(i, j) +=
                time_step * (EquationData::g_lam / EquationData::g_c_T *
                             grad_phi_i_T * grad_phi_j_T * fe_values.JxW(q));
            T_local_convection_matrix(i, j) +=
                time_step * EquationData::g_c_w / EquationData::g_c_T *
                phi_i_T *
                (-EquationData::g_perm *
                 (old_P_sol_grads[q] +
                  (Point<dim>(0, 0, 1)) * EquationData::g_P_grad) *
                 grad_phi_j_T * fe_values.JxW(q));
          }

          T_local_rhs(i) +=
              (time_step * T_source_values[q] + old_T_sol_values[q]) * phi_i_T *
              fe_values.JxW(q);
        }
      }

      // APPLIED NEUMAN BOUNDARY CONDITION
      for (int bd_i = 0; bd_i < EquationData::g_num_QT_bnd_id; bd_i++) {
        for (unsigned int face_no = 0;
             face_no < GeometryInfo<dim>::faces_per_cell; ++face_no) {
          if (cell->at_boundary(face_no) &&
              cell->face(face_no)->boundary_id() ==
                  EquationData::g_QT_bnd_id[bd_i]) {
            fe_face_values.reinit(cell, face_no);

            // get boundary condition
            QT_boundary.set_time(time);
            QT_boundary.set_boundary_id(*(EquationData::g_QT_bnd_id + bd_i));
            QT_boundary.value_list(fe_face_values.get_quadrature_points(),
                                   QT_bd_values);

            for (unsigned int q = 0; q < n_face_q_points; ++q) {

              const auto T_face_quadrature_coord =
                  fe_face_values.quadrature_point(q);
              //edit by Yuan 07/22/2020 delete
              // EquationData::g_perm =
              //     interpolate1d(EquationData::g_perm_list,
              //                   T_face_quadrature_coord[2], false);  // step-5
              //edit by Yuan 07/22/2020 add
              EquationData::g_perm = data_interpolation.value(T_face_quadrature_coord[0], T_face_quadrature_coord[1], T_face_quadrature_coord[2]);

              for (unsigned int i = 0; i < dofs_per_cell; ++i) {
                T_local_rhs(i) += -time_step / EquationData::g_c_T *
                                  fe_face_values.shape_value(i, q) *
                                  QT_bd_values[q] * fe_face_values.JxW(q);
              }
            }
          }
        }
      }
      // local ->globe
      cell->get_dof_indices(T_local_dof_indices);
      for (unsigned int i = 0; i < dofs_per_cell; ++i) {
        for (unsigned int j = 0; j < dofs_per_cell; ++j) {
          T_system_matrix.add(T_local_dof_indices[i], T_local_dof_indices[j],
                              T_local_mass_matrix(i, j));  // mass_matrix
          T_system_matrix.add(
              T_local_dof_indices[i], T_local_dof_indices[j],
              T_local_stiffness_matrix(i, j));  // striffness_matrix
          T_system_matrix.add(
              T_local_dof_indices[i], T_local_dof_indices[j],
              T_local_convection_matrix(i, j));  // convection_matrix
        }
        T_system_rhs(T_local_dof_indices[i]) += T_local_rhs(i);
      }
    }
  }

  // compress the matrix
  T_system_matrix.compress(VectorOperation::add);
  T_system_rhs.compress(VectorOperation::add);

  // ADD DIRICHLET BOUNDARY
  {

    for (int bd_i = 0; bd_i < EquationData::g_num_T_bnd_id; bd_i++) {

      T_boundary.set_time(time);
      T_boundary.set_boundary_id(*(EquationData::g_T_bnd_id + bd_i));
      std::map<types::global_dof_index, double> T_bd_values;
      VectorTools::interpolate_boundary_values(
          dof_handler, *(EquationData::g_T_bnd_id + bd_i), T_boundary,
          T_bd_values);  // i表示边界的index
      MatrixTools::apply_boundary_values(T_bd_values, T_system_matrix,
                                         T_solution, T_system_rhs, false);
    }
  }

  timer.tock("assemble_T_system");
}

template <int dim>
void CoupledTH<dim>::linear_solve_P() {
  cbgeo::Clock timer;
  timer.tick();
  SolverControl solver_control(
      P_solution.size(),
      P_tol_residual * P_system_rhs.l2_norm());  // setting for cg
  PETScWrappers::SolverCG cg(solver_control, mpi_communicator);  // config cg
  PETScWrappers::PreconditionBlockJacobi preconditioner(
      P_system_matrix);  // use a block Jacobi preconditioner which works by
                         // computing an incomplete LU decomposition on each
                         // diagonal block of the matrix. (In other words, each
                         // MPI process computes an ILU from the rows it stores
                         // by throwing away columns that correspond to row
                         // indices not stored locally; this yields a square
                         // matrix block from which we can compute an ILU.
  cg.solve(P_system_matrix, P_solution, P_system_rhs,
           preconditioner);  // solve eq
  P_iteration_namber = solver_control.last_step();

  pcout << "\nCG iterations: " << P_iteration_namber << std::endl;

  timer.tock("linear_solve_P");
}

template <int dim>
void CoupledTH<dim>::linear_solve_T() {
  cbgeo::Clock timer;
  timer.tick();
  SolverControl solver_control(
      std::max<std::size_t>(n_T_max_iteration, T_system_rhs.size() / 10),
      T_tol_residual * T_system_rhs.l2_norm());  // setting for solver
  PETScWrappers::SolverGMRES solver(solver_control,
                                    mpi_communicator);  // config solver
  PETScWrappers::PreconditionJacobi preconditioner(T_system_matrix);  // precond
  // preconditioner.initialize(T_system_matrix, 1.0);      // initialize precond
  solver.solve(T_system_matrix, T_solution, T_system_rhs,
               preconditioner);  // solve eq

  T_iteration_namber = solver_control.last_step();

  pcout << " \n Iterations required for convergence:    " << T_iteration_namber
        << "\n";

  // constraints.distribute(solution);  // make sure if the value is
  // consistent at the constraint point

  timer.tock("linear_solve_T");
}

// @sect4{<code>CoupledTH::output_results</code>}
//
// Neither is there anything new in generating graphical output:
template <int dim>
void CoupledTH<dim>::output_results(PETScWrappers::MPI::Vector& solution,
                                    std::string var_name) const {

  const Vector<double> localized_solution(solution);

  if (this_mpi_process == 0) {
    const std::string filename = "outputfiles/" + var_name + "-solution-" +
                                 Utilities::int_to_string(timestep_number, 3) +
                                 ".vtk";
    std::ofstream output(filename.c_str());

    DataOut<dim> data_out;
    data_out.attach_dof_handler(dof_handler);
    data_out.add_data_vector(localized_solution, var_name);

    // The only other thing we do here is that we also output one
    // value per cell indicating which subdomain (i.e., MPI
    // process) it belongs to. This requires some conversion work,
    // since the data the library provides us with is not the one
    // the output class expects, but this is not difficult. First,
    // set up a vector of integers, one per cell, that is then
    // filled by the subdomain id of each cell.
    //
    // The elements of this vector are then converted to a
    // floating point vector in a second step, and this vector is
    // added to the DataOut object, which then goes off creating
    // output in VTK format:
    std::vector<unsigned int> partition_int(triangulation.n_active_cells());
    GridTools::get_subdomain_association(triangulation, partition_int);

    const Vector<double> partitioning(partition_int.begin(),
                                      partition_int.end());

    data_out.add_data_vector(partitioning, "partitioning");

    data_out.build_patches();
    data_out.write_vtk(output);
  }
}

template <int dim>
void CoupledTH<dim>::run() {

  unsigned int binary_search_number;
  double initial_time_step;
  double theta;

  make_grid();

  setup_system();

  pcout << "   Number of degrees of freedom: " << dof_handler.n_dofs()
        << " (by partition:";
  for (unsigned int process = 0; process < n_mpi_processes; ++process) {
    pcout << (process == 0 ? ' ' : '+')
          << (DoFTools::count_dofs_with_subdomain_association(dof_handler,
                                                              process));
  }
  pcout << ")" << std::endl;

  VectorTools::interpolate(dof_handler,
                           EquationData::TemperatureInitialValues<dim>(),
                           old_T_solution);
  VectorTools::interpolate(
      dof_handler, EquationData::PressureInitialValues<dim>(), old_P_solution);

  output_results(old_T_solution, "T");
  output_results(old_P_solution, "P");

  do {
    pcout << "\nTimestep " << timestep_number;

    binary_search_number = 1;
    initial_time_step =
        time_sequence[timestep_number + 1] - time_sequence[timestep_number];
    time_step = initial_time_step / 2;
    theta = 0;

    do {

      assemble_P_system();

      linear_solve_P();

      old_P_solution = P_solution;

      assemble_T_system();

      linear_solve_T();

      old_T_solution = T_solution;

      time += time_step;

      theta += pow(0.5, binary_search_number);

      if (P_iteration_namber > n_P_max_iteration / 2 ||
          T_iteration_namber > n_T_max_iteration / 2) {
        time_step = time_step / 2;
        ++binary_search_number;
      }

      pcout << "\nt=" << time << ", dt=" << time_step << '.' << std::endl;

    } while ((1 - theta) > 0.00001);

    timestep_number += 1;
    output_results(T_solution, "T");
    output_results(P_solution, "P");

    pcout << "\n" << std::endl << std::endl;

    // MatrixOut matrix_out;
    // std::ofstream out ("2rhs_matrix_at_"+std::to_string(time));
    // // matrix_out.build_patches (system_matrix, "system_matrix");
    // // matrix_out.write_gnuplot (out);
    // // system_matrix.print_formatted(out);
    // system_rhs.print(out);

  } while (time < period);
}
