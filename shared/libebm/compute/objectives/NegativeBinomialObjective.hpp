// Copyright (c) 2023 The InterpretML Contributors
// Licensed under the MIT license.
// Author: Paul Koch <code@koch.ninja>

#include <cmath>   // For std::exp, std::log, std::lgamma, std::isnan, std::isinf
#include <limits>  // For std::numeric_limits

// !! To add a new objective in C++ follow the steps at the top of the "objective_registrations.hpp" file !!

// TFloat is a wrapper that could hold a double, float, or SIMD intrinsic type. It can also expose GPU operations.
// See cpu_64.cpp, avx2_32.cpp, and cuda_32.cu as examples where TFloat operators are defined.
template<typename TFloat> struct NegativeBinomialObjective : RegressionObjective { // MODIFIED class name
   // Parameters for the OBJECTIVE_BOILERPLATE are:
   //   - this class type
   //   - MINIMIZE_METRIC or MAXIMIZE_METRIC determines which direction the metric should go for early stopping
   //   - Link function type. See libebm.h for a list of available link functions
   OBJECTIVE_BOILERPLATE(NegativeBinomialObjective, MINIMIZE_METRIC, Objective_Other, Link_log, true) // MODIFIED parameters

   // member variables should be of type TFloat
   TFloat m_alpha; // MODIFIED to store alpha
   TFloat inv_alpha;

   // constexpr values should be static and type double
   static constexpr double Two = 2.0;

   // The constructor parameters following config must match the RegisterObjective parameters in
   // objective_registrations.hpp
   inline NegativeBinomialObjective(const Config& config, const double alpha) { // MODIFIED parameters
      if(config.cOutputs != 1) {
         throw ParamMismatchWithConfigException();
      }

      if(config.isDifferentialPrivacy) {
         // Do not support differential privacy unless this objective has been mathematically proven to work in DP
         throw NonPrivateRegistrationException();
      }

      m_alpha = alpha; // Store alpha
      inv_alpha = 1.0 / m_alpha; // Calculate inverse alpha
   }

   inline bool CheckRegressionTarget(const double target) const noexcept {
      return std::isnan(target) || std::isinf(target);
   }

   inline double LinkParam() const noexcept {
      // only Link_power and the custom link functions use the LinkParam
      return std::numeric_limits<double>::quiet_NaN();
   }

   inline double LearningRateAdjustmentDifferentialPrivacy() const noexcept {
      // WARNING: Do not change this rate without accounting for it in the privacy budget if this objective supports DP
      return 1.0;
   }

   inline double LearningRateAdjustmentGradientBoosting() const noexcept {
      return 1.0; // typically leave this at 1.0 (unmodified)
   }

   inline double LearningRateAdjustmentHessianBoosting() const noexcept {
      return 1.0; // typically leave this at 1.0 (unmodified)
   }

   inline double GainAdjustmentGradientBoosting() const noexcept {
      return 1.0; // typically leave this at 1.0 (unmodified)
   }

   inline double GainAdjustmentHessianBoosting() const noexcept {
      return 1.0; // typically leave this at 1.0 (unmodified)
   }

   inline double GradientConstant() const noexcept {
      return 1.0; // as a speed optimization, any constant multiples in CalcGradientHessian can be moved here
   }

   inline double HessianConstant() const noexcept {
      return 1.0; // as a speed optimization, any constant multiples in CalcGradientHessian can be moved here
   }

   inline double FinishMetric(const double metricSum) const noexcept {
      return metricSum; // return MSE in this example, but if we wanted to return RMSE we would take the sqrt here
   }

   GPU_DEVICE inline TFloat CalcMetric(const TFloat& score, const TFloat& target) const noexcept {
      // Current calculation is for MSE (identity link), not using m_alpha or Link_log.
      // This is a placeholder as per user request.
      const TFloat prediction = Exp(score); 
      const TFloat part1 = target + m_alpha * prediction;
      const TFloat part2 = Log( part1 );
      // const TFloat error = prediction - target;
      // const TFloat error = ( target+ 1/ m_alpha ) * Log( 1 + m_alpha *prediction) - target * Log(prediction);
       const TFloat error = ( target + inv_alpha  ) * part2   - target * Log(prediction);
      return error;
   }

   GPU_DEVICE inline TFloat CalcGradient(const TFloat& score, const TFloat& target) const noexcept {
      const TFloat prediction = Exp(score); // log link function
      const TFloat gradient = prediction - target;
      return gradient;
   }

   GPU_DEVICE inline GradientHessian<TFloat> CalcGradientHessian(
         const TFloat& score, const TFloat& target) const noexcept {
      const TFloat prediction = Exp(score); // log link function
      const TFloat gradient = prediction - target;
      const TFloat hessian = prediction + m_alpha * prediction * prediction; // hessian = prediction + alpha * prediction^2
      return MakeGradientHessian(gradient, hessian);
   }
};
