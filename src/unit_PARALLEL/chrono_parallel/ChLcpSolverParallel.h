#ifndef CHC_LCPITERATIVESOLVERGPU_H
#define CHC_LCPITERATIVESOLVERGPU_H

//////////////////////////////////////////////////
//
//   ChIterativeGPU.h
//
//   GPU LCP Solver
//
//   HEADER file for CHRONO,
//   Multibody dynamics engine
//
// ------------------------------------------------
//   Copyright:Alessandro Tasora / DeltaKnowledge
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////

#include "lcp/ChLcpIterativeSolver.h"

#include "chrono_parallel/ChParallelDefines.h"
#include "chrono_parallel/ChDataManager.h"
#include "chrono_parallel/ChIntegratorParallel.h"
#include "chrono_parallel/constraints/ChConstraintRigidRigid.h"
#include "chrono_parallel/constraints/ChConstraintBilateral.h"
#include "chrono_parallel/math/ChParallelMath.h"

#include "chrono_parallel/solver/ChSolverAPGD.h"
#include "chrono_parallel/solver/ChSolverAPGDRS.h"
#include "chrono_parallel/solver/ChSolverBiCG.h"
#include "chrono_parallel/solver/ChSolverBiCGStab.h"
#include "chrono_parallel/solver/ChSolverCG.h"
#include "chrono_parallel/solver/ChSolverCGS.h"
#include "chrono_parallel/solver/ChSolverMinRes.h"
#include "chrono_parallel/solver/ChSolverSD.h"
#include "chrono_parallel/solver/ChSolverGD.h"

namespace chrono {

class CH_PARALLEL_API ChLcpSolverParallel : public ChLcpIterativeSolver {
 public:
   ChLcpSolverParallel() {
      tolerance = 1e-7;
      max_iter_bilateral = 100;
      record_violation_history = true;
      warm_start = false;
   }

   virtual ~ChLcpSolverParallel() {
   }

   virtual double Solve(
                        ChLcpSystemDescriptor &sysd) {
      return 0;
   }
   void host_addForces(
                       bool* active,
                       real *mass,
                       real3 *inertia,
                       real3 *forces,
                       real3 *torques,
                       real3 *vel,
                       real3 *omega);

   void host_ComputeGyro(
                         real3 *omega,
                         real3 *inertia,
                         real3 *gyro,
                         real3 *torque);

   virtual void RunTimeStep(
                            real step) = 0;
   void Preprocess();

   void SetTolerance(
                     real tol) {
      tolerance = tol;
   }

   void SetMaxIterationBilateral(
                                 uint max_iter) {
      max_iter_bilateral = max_iter;
   }
   real GetResidual() {
      return residual;
   }

   ChParallelDataManager *data_container;

 protected:
   real tolerance;
   real residual;

   uint max_iter_bilateral;

   ChConstraintBilateral bilateral;
};

class CH_PARALLEL_API ChLcpSolverParallelDVI : public ChLcpSolverParallel {
 public:
   ChLcpSolverParallelDVI() {
      alpha = .2;
      contact_recovery_speed = .6;
      do_stab = false;
      collision_inside = false;
      update_rhs = false;

      max_iteration = 1000;
      max_iter_normal = max_iter_sliding = max_iter_spinning = 100;

      solver = new ChSolverAPGD();

   }

   virtual void RunTimeStep(
                            real step);
   void RunWarmStartPostProcess();
   void RunWarmStartPreprocess();

   void SetCompliance(
                      real a) {
      data_container->alpha = a;
   }
   void SetSolverType(
                      GPUSOLVERTYPE type) {
      solver_type = type;

      if (this->solver) {
         delete (this->solver);
      }

      if (solver_type == STEEPEST_DESCENT) {
         solver = new ChSolverSD();
      } else if (solver_type == GRADIENT_DESCENT) {
         solver = new ChSolverGD();
      } else if (solver_type == CONJUGATE_GRADIENT) {
         solver = new ChSolverCG();
      } else if (solver_type == CONJUGATE_GRADIENT_SQUARED) {
         solver = new ChSolverCGS();
      } else if (solver_type == BICONJUGATE_GRADIENT) {
         solver = new ChSolverBiCG();
      } else if (solver_type == BICONJUGATE_GRADIENT_STAB) {
         solver = new ChSolverBiCGStab();
      } else if (solver_type == MINIMUM_RESIDUAL) {
         solver = new ChSolverMinRes();
      } else if (solver_type == QUASAI_MINIMUM_RESIDUAL) {
         //         // This solver has not been implemented yet
         //         //SolveQMR(data_container->gpu_data.device_gam_data, rhs, max_iteration);
      } else if (solver_type == APGD) {
         solver = new ChSolverAPGD();
      } else if (solver_type == APGDRS) {
         solver = new ChSolverAPGDRS();
      }

   }
   void SetMaxIterationNormal(
                              uint max_iter) {
      max_iter_normal = max_iter;
   }
   void SetMaxIterationSliding(
                               uint max_iter) {
      max_iter_sliding = max_iter;
   }
   void SetMaxIterationSpinning(
                                uint max_iter) {
      max_iter_spinning = max_iter;
   }
   void SetMaxIteration(
                        uint max_iter) {
      max_iteration = max_iter;
      max_iter_normal = max_iter_sliding = max_iter_spinning = max_iter_bilateral = max_iter;
   }
   void SetContactRecoverySpeed(
                                real recovery_speed) {
      data_container->contact_recovery_speed = fabs(recovery_speed);
   }
   void DoStabilization(
                        bool stab) {
      do_stab = stab;
   }
   void DoCollision(
                    bool do_collision) {
      collision_inside = do_collision;
   }
   void DoUpdateRHS(
                    bool do_update_rhs) {
      update_rhs = do_update_rhs;
   }
   ChSolverParallel *solver;

 private:
   GPUSOLVERTYPE solver_type;

   real alpha;

   real contact_recovery_speed;
   bool do_stab;
   bool collision_inside;
   bool update_rhs;

   uint max_iteration;
   uint max_iter_normal;
   uint max_iter_sliding;
   uint max_iter_spinning;

   ChConstraintRigidRigid rigid_rigid;
};

class CH_PARALLEL_API ChLcpSolverParallelDEM : public ChLcpSolverParallel {
 public:

   ChLcpSolverParallelDEM(){
      solver = new ChSolverAPGD();
   }

   virtual void RunTimeStep(
                            real step);

   void SetMaxIteration(
                        uint max_iter) {
      max_iter_bilateral = max_iter;
   }

   void ProcessContacts();

 private:
   void host_CalcContactForces(
                               custom_vector<int>& ext_body_id,
                               custom_vector<real3>& ext_body_force,
                               custom_vector<real3>& ext_body_torque);

   void host_AddContactForces(
                              uint ct_body_count,
                              const custom_vector<int>& ct_body_id,
                              const custom_vector<real3>& ct_body_force,
                              const custom_vector<real3>& ct_body_torque);

   ChSolverParallel * solver;
};

}
// end namespace chrono

#endif

