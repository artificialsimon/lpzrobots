#include "diamond.h"
#include "soxdiamond.h"
#include <iterator>
#include <algorithm>
using namespace matrix;
using namespace std;

Diamond::Diamond(const DiamondConf& conf)
  : AbstractController("Diamond", "0.1"),
    conf(conf)
{
  t=0;


  //addInspectableMatrix("A", &A, conf.someInternalParams, "model matrix");
  //if(conf.useExtendedModel)
    //addInspectableMatrix("S", &S, conf.someInternalParams, "model matrix (sensor branch)");
  //addInspectableMatrix("C", &C, conf.someInternalParams, "controller matrix");
  //addInspectableMatrix("L", &L, conf.someInternalParams, "Jacobi matrix");
  //addInspectableMatrix("h", &h, conf.someInternalParams, "controller bias");
  //addInspectableMatrix("b", &b, conf.someInternalParams, "model bias");
  //addInspectableMatrix("R", &R, conf.someInternalParams, "linear response matrix");

  //addInspectableMatrix("v_avg", &v_avg, "input shift (averaged)");


};

Diamond::~Diamond(){
}


void Diamond::init(int sensornumber, int motornumber, RandGen* randGen){
  if(!randGen) randGen = new RandGen(); // this gives a small memory leak

  number_sensors= sensornumber;
  number_motors = motornumber;

  for (int i = 0; i < conf.n_layers; i++) {
    // TODO still desiding how to implement Diamond layers
    //Sox* one_layer = make_layer(conf.base_controller_name);
    //one_layer->init(sensornumber, motornumber, randGen);
    internal_layer.push_back(make_layer(conf.base_controller_name));
    internal_layer[i]->init(sensornumber, motornumber, randGen);
    //SoxConf oll_conf = Sox::getDefaultConf();
    //Sox* ol = new Sox(oll_conf);
    //oll = new Sox(Sox::getDefaultConf());
    //internal_layer.push_back(one_layer);
    //addInspectableMatrix("A", internal_layer[i]->getpA(), conf.someInternalParams, "model matrix");
    //addInspectableMatrix("A", &(ol->getA()), conf.someInternalParams, "model matrix");
    //addInspectableMatrix("A", &(oll.getA()), conf.someInternalParams, "model matrix");
  }



  // TODO delete from here down
  //A.set(number_sensors, number_motors);
  //S.set(number_sensors, number_sensors);
  //C.set(number_motors, number_sensors);
  //b.set(number_sensors, 1);
  //h.set(number_motors, 1);
  //L.set(number_sensors, number_sensors);
  //v_avg.set(number_sensors, 1);
  //A_native.set(number_sensors, number_motors);
  //C_native.set(number_motors, number_sensors);

  //R.set(number_sensors, number_sensors);


  //C.toId(); // set a to identity matrix;
  ////C*=conf.initFeedbackStrength;

  //S.toId();
  //S*=0.05;

  //// if motor babbling is used then this is overwritten
  //A_native.toId();
  //C_native.toId();
  //C_native*=1.2;

  //y_teaching.set(number_motors, 1);

  //x.set(number_sensors,1);
  //x_smooth.set(number_sensors,1);
  //for (unsigned int k = 0; k < buffersize; k++) {
    //x_buffer[k].set(number_sensors,1);
    //y_buffer[k].set(number_motors,1);

  //}
}

// performs one step (includes learning). Calculates motor commands from sensor inputs.
void Diamond::step(const sensor* x_, int number_sensors,
                       motor* y_, int number_motors){
  if (conf.n_layers == 1) {  // No Diamond, only HK
      internal_layer[0]->step(x_, number_sensors, y_, number_motors);
  } 
  else {
    Matrix x_l;  // Sensor values to be used in stepMV
    for (int i = 0; i < conf.n_layers; i++) {
      if (i == 0) {
        x.set(number_sensors,1,x_); // x^prime_0 
        x_l = x;  // First layer it is copy
      }
      else if (i == 1) {
        x.set(number_sensors,1,x_); // x^prime_0 
        x_l = (internal_layer[i-1]->getC()^T) * (
            ((internal_layer[i-1]->getA()^T) * (x - internal_layer[i-1]->getb())).map(g_inv) - internal_layer[i-1]->geth());
      }
      else {
        // Internal world step
        matrix::Matrix internal_x = internal_layer[i-2]->getA() 
          * internal_layer[i-2]->getLastMotorValues()
          + internal_layer[i-2]->getb();  // x^prime_i
        x_l = (internal_layer[i-1]->getC()^T) *
          (
           ((internal_layer[i-1]->getA()^T) *
            (internal_x - internal_layer[i-1]->getb())
            ).map(g_inv)
           - internal_layer[i-1]->geth()
           );
      }
      sensor x_pw[number_sensors];
      x_l.convertToBuffer(x_pw, number_sensors);
      motor y_discard[number_motors];
      if (i < conf.n_layers - 1) {
        internal_layer[i]->stepMV(x_pw, number_sensors, y_discard, number_motors,
            internal_layer[i+1]);
        if (i == 0) {
          for (int j = 0; j < number_motors; j++)
            y_[j] = y_discard[j]; 
        }
      }
      else {
        // Last layer is only HK for motors command
        internal_layer[i]->step(x_pw, number_sensors, y_discard, number_motors);
      }
    }
  }
  t++;
};


SoxDiamond* Diamond::make_layer(string name){
  return new SoxDiamond();
}

matrix::Matrix Diamond::getA(){
  return A;
}

void Diamond::setA(const matrix::Matrix& _A){
  assert(A.getM() == _A.getM() && A.getN() == _A.getN());
  A=_A;
}

matrix::Matrix Diamond::getC(){
  return C;
}

void Diamond::setC(const matrix::Matrix& _C){
  assert(C.getM() == _C.getM() && C.getN() == _C.getN());
  C=_C;
}

matrix::Matrix Diamond::geth(){
  return h;
}

void Diamond::seth(const matrix::Matrix& _h){
  assert(h.getM() == _h.getM() && h.getN() == _h.getN());
  h=_h;
}

// performs one step without learning. Calulates motor commands from sensor inputs.
void Diamond::stepNoLearning(const sensor* x_, int number_sensors,
                                 motor* y_, int number_motors){
  assert((unsigned)number_sensors <= this->number_sensors
         && (unsigned)number_motors <= this->number_motors);

  //x.set(number_sensors,1,x_); // store sensor values

  //// averaging over the last s4avg values of x_buffer
  //conf.steps4Averaging = ::clip(conf.steps4Averaging,1,buffersize-1);
  //if(conf.steps4Averaging > 1)
    //x_smooth += (x - x_smooth)*(1.0/conf.steps4Averaging);
  //else
    //x_smooth = x;

  //x_buffer[t%buffersize] = x_smooth; // we store the smoothed sensor value

  //// calculate controller values based on current input values (smoothed)
  //Matrix y =   (C*(x_smooth + (v_avg*creativity)) + h).map(g);

  //// Put new output vector in ring buffer y_buffer
  //y_buffer[t%buffersize] = y;

  //// convert y to motor*
  //y.convertToBuffer(y_, number_motors);

  //update step counter
  //t++;
};


Matrix Diamond::pseudoInvL(const Matrix& L, const Matrix& A, const Matrix& C){
  if(pseudo == 0){
    return L.pseudoInverse();
  }else{
    const Matrix& P = pseudo==1 || pseudo==2 ? A^T : C;
    const Matrix& Q = pseudo==1              ? C^T : A;
    return Q *((P * L * Q)^(-1)) * P;
  }
}


// learn values h,C,A,b,S
void Diamond::learn(){


  //// the effective x/y is (actual-steps4delay) element of buffer
  //int s4delay = ::clip(conf.steps4Delay,1,buffersize-1);
  //const Matrix& x       = x_buffer[(t - max(s4delay,1) + buffersize) % buffersize];
  //const Matrix& y_creat = y_buffer[(t - max(s4delay,1) + buffersize) % buffersize];
  //const Matrix& x_fut   = x_buffer[t% buffersize]; // future sensor (with respect to x,y)

  //const Matrix& xi      = x_fut  - (A * y_creat + b + S * x); // here we use creativity

  //const Matrix& z       = (C * (x) + h); // here no creativity
  //const Matrix& y       = z.map(g);
  //const Matrix& g_prime = z.map(g_s);

  //L = A * (C & g_prime) + S;
  //R = A * C+S; // this is only used for visualization

  //const Matrix& eta    = A.pseudoInverse() * xi;
  //const Matrix& y_hat  = y + eta*causeaware;

  //const Matrix& Lplus  = pseudoInvL(L,A,C);
  //const Matrix& v      = Lplus * xi;
  //const Matrix& chi    = (Lplus^T) * v;

  //const Matrix& mu     = ((A^T) & g_prime) * chi;
  //const Matrix& epsrel = (mu & (C * v)) * (sense * 2);

  //const Matrix& v_hat = v + x * harmony;

  //v_avg += ( v  - v_avg ) *.1;

  //double EE = 1.0;
  //if(loga){
    //EE = .1/(v.norm_sqr() + .001); // logarithmic error (E = log(v^T v))
  //}
  //if(epsA > 0){
    //double epsS=epsA*conf.factorS;
    //double epsb=epsA*conf.factorb;
    //A   += (xi * (y_hat^T) * epsA                      ).mapP(0.1, clip);
    //if(damping)
      //A += (((A_native-A).map(power3))*damping         ).mapP(0.1, clip);
    //if(conf.useExtendedModel)
      //S += (xi * (x^T)     * (epsS)+ (S *  -damping*10) ).mapP(0.1, clip);
    //b   += (xi             * (epsb) + (b *  -damping)    ).mapP(0.1, clip);
  //}
  //if(epsC > 0){
    //C += (( mu * (v_hat^T)
            //- (epsrel & y) * (x^T))   * (EE * epsC) ).mapP(.05, clip);
    //if(damping)
      //C += (((C_native-C).map(power3))*damping      ).mapP(.05, clip);
    //h += ((mu*harmony - (epsrel & y)) * (EE * epsC * conf.factorh) ).mapP(.05, clip);

    //if(intern_isTeaching && gamma > 0){
      //// scale of the additional terms
      //const Matrix& metric = (A^T) * Lplus.multTM() * A;

      //const Matrix& y      = y_buffer[(t-1)% buffersize];
      //const Matrix& xsi    = y_teaching - y;
      //const Matrix& delta  = xsi.multrowwise(g_prime);
      //C += ((metric * delta*(x^T) ) * (gamma * epsC)).mapP(.05, clip);
      //h += ((metric * delta)        * (gamma * epsC * conf.factorh)).mapP(.05, clip);
      //// after we applied teaching signal it is switched off until new signal is given
      //intern_isTeaching    = false;
    //}
  //}

};


void Diamond::setMotorTeaching(const matrix::Matrix& teaching){
  assert(teaching.getM() == number_motors && teaching.getN() == 1);
  // Note: through the clipping the otherwise effectless
  //  teaching with old motor value has now an effect,
  //  namely to drive out of the saturation region.
  //y_teaching= teaching.mapP(0.95,clip);
  //intern_isTeaching=true;
}

void Diamond::setSensorTeaching(const matrix::Matrix& teaching){
  assert(teaching.getM() == number_sensors && teaching.getN() == 1);
  // calculate the y_teaching,
  // that belongs to the distal teaching value by the inverse model.
  //y_teaching = (A.pseudoInverse() * (teaching-b)).mapP(0.95, clip);
  //intern_isTeaching=true;
}

matrix::Matrix Diamond::getLastMotorValues(){
  return y_buffer[(t-1+buffersize)%buffersize];
}

matrix::Matrix Diamond::getLastSensorValues(){
  return x_buffer[(t-1+buffersize)%buffersize];
}

list<Matrix> Diamond::getParameters() const {
  return {C,h};
}

int Diamond::setParameters(const list<Matrix>& params){
  if(params.size() == 2){
    list<Matrix>::const_iterator i = params.begin();
    const Matrix& CN = *i;
    if(C.hasSameSizeAs(CN)) C=CN;
    else return false;
    const Matrix& hN = *(++i);
    if(h.hasSameSizeAs(hN)) h=hN;
    else return false;
  } else {
    fprintf(stderr,"setParameters wrong len %i!=2\n", (int)params.size());
    return false;
  }
  return true;
}

/* stores the controller values to a given file. */
bool Diamond::store(FILE* f) const{
  // save matrix values
  C.store(f);
  h.store(f);
  A.store(f);
  b.store(f);
  S.store(f);
  Configurable::print(f,0);
  return true;
}

/* loads the controller values from a given file. */
bool Diamond::restore(FILE* f){
  // save matrix values
  C.restore(f);
  h.restore(f);
  A.restore(f);
  b.restore(f);
  S.restore(f);
  Configurable::parse(f);
  t=0; // set time to zero to ensure proper filling of buffers
  return true;
}
