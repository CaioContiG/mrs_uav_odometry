#include "StateEstimator.h"

namespace mrs_odometry
{

/*  //{ StateEstimator() */

// clang-format off
StateEstimator::StateEstimator(
    const std::string &estimator_name,
    const std::vector<bool> &fusing_measurement,
    const std::vector<Eigen::MatrixXd> &P_arr,
    const std::vector<Eigen::MatrixXd> &Q_arr,
    const Eigen::MatrixXd &A,
    const Eigen::MatrixXd &B,
    const Eigen::MatrixXd &R)
    :
    m_estimator_name(estimator_name),
    m_fusing_measurement(fusing_measurement),
    m_P_arr(P_arr),
    m_Q_arr(Q_arr),
    m_A(A),
    m_B(B),
    m_R(R)
  {

  Eigen::MatrixXd Q_zero = Eigen::MatrixXd::Zero(1, 1);
  Eigen::MatrixXd P_zero = Eigen::MatrixXd::Zero(1, m_A.rows());

  mp_lkf_x = new mrs_lib::Lkf(m_A.rows(), 1, 1, m_A, m_B, m_R, Q_zero, P_zero);
  mp_lkf_y = new mrs_lib::Lkf(m_A.rows(), 1, 1, m_A, m_B, m_R, Q_zero, P_zero);

  std::cout << "[StateEstimator]: New StateEstimator initialized " << std::endl;
  std::cout << "name: " << m_estimator_name << std::endl;
  std::cout << " fusing measurements: " << std::endl;
  for(size_t i=0;i<m_fusing_measurement.size();i++){std::cout << m_fusing_measurement[i] << " ";}
  std::cout << std::endl << " P_arr: " << std::endl;
  for(size_t i=0;i<m_P_arr.size();i++){std::cout << m_P_arr[i] << std::endl;}
  std::cout << std::endl << " Q_arr: " << std::endl;
  for(size_t i=0;i<m_Q_arr.size();i++){std::cout << m_Q_arr[i] << std::endl;}
  std::cout << std::endl << " A: " << std::endl << m_A << std::endl << " B: " << std::endl << m_B << std::endl << " R: " << std::endl << m_R << std::endl;
}
// clang-format on

//}

/*  //{ doPrediction() */

void StateEstimator::doPrediction(const Eigen::VectorXd &input, double dt) {

  /* std::cout << "[StateEstimator]: " << m_estimator_name << " fusing input: " << input << " with time step: " << dt << std::endl; */

  Eigen::VectorXd input_vec_x = Eigen::VectorXd::Zero(1);
  Eigen::VectorXd input_vec_y = Eigen::VectorXd::Zero(1);

  input_vec_x << input(0);
  input_vec_y << input(1);

  Eigen::MatrixXd newA = m_A;
  newA(0, 1)           = dt;
  newA(1, 2)           = dt;

  mutex_lkf.lock();
  {
    mp_lkf_x->setA(newA);
    mp_lkf_x->setInput(input_vec_x);
    mp_lkf_x->iterateWithoutCorrection();
    mp_lkf_y->setA(newA);
    mp_lkf_y->setInput(input_vec_y);
    mp_lkf_y->iterateWithoutCorrection();
  }
  mutex_lkf.unlock();
}

//}

/*  //{ doCorrection() */

void StateEstimator::doCorrection(const Eigen::VectorXd &measurement, int measurement_type) {

  /* std::cout << "[StateEstimator]: " << m_estimator_name << " fusing correction: " << measurement << " of type: " << measurement_type << std::endl; */

  if (measurement_type > (int)m_fusing_measurement.size()) {
    std::cerr << "[StateEstimator]: Tried to fuse invalid measurement type: " << measurement_type << std::endl;
    return;
  }

  if (!m_fusing_measurement[measurement_type]) {
    return;
  }

  Eigen::VectorXd mes_vec_x = Eigen::VectorXd::Zero(1);
  Eigen::VectorXd mes_vec_y = Eigen::VectorXd::Zero(1);

  mes_vec_x << measurement(0);
  mes_vec_y << measurement(1);

  mutex_lkf.lock();
  {
    mp_lkf_x->setP(m_P_arr[measurement_type]);
    mp_lkf_x->setMeasurement(mes_vec_x, m_Q_arr[measurement_type]);
    mp_lkf_x->doCorrection();
    mp_lkf_y->setP(m_P_arr[measurement_type]);
    mp_lkf_y->setMeasurement(mes_vec_y, m_Q_arr[measurement_type]);
    mp_lkf_y->doCorrection();
  }
  mutex_lkf.unlock();
}

//}

/*  //{ getStates() */

Eigen::MatrixXd StateEstimator::getStates(void) {

  Eigen::MatrixXd states = Eigen::MatrixXd::Zero(m_A.rows(), m_A.rows());

  mutex_lkf.lock();
  {
    states.col(0) = mp_lkf_x->getStates();
    states.col(1) = mp_lkf_y->getStates();
  }
  mutex_lkf.unlock();

  return states;
}

//}

/*  //{ getState() */

double StateEstimator::getState(int row, int col) {
  double state;
  if (col == 0) {
    mutex_lkf.lock();
    { state = mp_lkf_x->getState(row); }
    mutex_lkf.unlock();
    return state;
  } else if (col == 1) {
    mutex_lkf.lock();
    { state = mp_lkf_y->getState(row); }
    mutex_lkf.unlock();
    return state;
  } else {
    std::cerr << "[StateEstimator]: Requested invalid state." << std::endl;
    return -1.0;
  }
}

//}

/*  //{ setState() */

bool StateEstimator::setState(int state_id, const Eigen::VectorXd &value) {

  if (state_id > m_A.rows()) {
    std::cerr << "[StateEstimator]: Requested setting invalid state: %d" << state_id;
    return false;
  }

  mutex_lkf.lock();
  {
    mp_lkf_x->setState(state_id, value(0));
    mp_lkf_y->setState(state_id, value(1));
  }
  mutex_lkf.unlock();
  return true;
}

//}

/*  //{ reset() */

void StateEstimator::reset(const Eigen::MatrixXd &states) {
  mutex_lkf.lock();
  {
    mp_lkf_x->reset(states.col(0));
    mp_lkf_y->reset(states.col(1));
  }
  mutex_lkf.unlock();
}

//}
}  // namespace mrs_odometry
