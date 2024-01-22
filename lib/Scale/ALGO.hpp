/*

*/
#ifndef ALGO_h
#define ALGO_h
#include "assert.h"
#include <math.h>
//INTERFACCIA ALGORITMO DI FILTRAGGIO
class ALGO 
{
    public:
    ALGO(){}

    virtual unsigned long filter(unsigned long measure)=0;
    virtual void free_mem();
};
//ALGORITMO DUMMY: RESTITUISCE LA MISURA
class Dummy:public ALGO{
    public:
    Dummy(){}
    unsigned long filter(unsigned long measure) {
        return measure;
    }
    void free_mem(){}
};

//ALGORITMO1: MEDIA MOBILE
class MediaMobile:public ALGO{
    private:
    unsigned int n_samples;
    unsigned long *samples;
    unsigned int i;
    void allocateMemory(void){
        assert(this->n_samples>0);
        this->samples=(unsigned long*) malloc(sizeof(unsigned long)*this->n_samples);
        assert(this->samples!=NULL);
        this->i=0;
    }

    public:
    MediaMobile(){
        this->n_samples=1;
        this->allocateMemory();
    }
    MediaMobile(unsigned int n_samples){
        this->n_samples=n_samples;
        this->allocateMemory();
    }
    unsigned long filter(unsigned long measure) {
        int j=0;
        unsigned long sum=0;
        if(this->i < this->n_samples) { //Vector not full
            samples[i]=measure;
            i++;
        }
        else { //Vector already full
            for(j=0; j<n_samples-1; j++){ //Left shift vector with new entrance
                this->samples[j]=this->samples[j+1];
            }
            this->samples[n_samples-1]=measure;
        }
        //Evaluate the mean
        for(j=0;j<this->i;j++) sum+=this->samples[j];
        return sum/this->i;
    }
    void free_mem(){
        free(this->samples);
    }
};

//ALGORITMO2: MEDIANA
class Mediana:public ALGO{
    private:
    unsigned int n_samples;
    unsigned long *samples;
    unsigned int i;
    void allocateMemory(void){
        assert(this->n_samples>0);
        this->samples=(unsigned long*) malloc(sizeof(unsigned long)*this->n_samples);
        assert(this->samples!=NULL);
        this->i=0;
    }

    public:
    Mediana(){
        this->n_samples=1;
        this->allocateMemory();
    }
    Mediana(unsigned int n_samples){
        this->n_samples=n_samples;
        this->allocateMemory();
    }
    unsigned long filter(unsigned long measure) {
        int j=0;
        if(this->i < this->n_samples) { //Vector not full
            samples[this->i]=measure;
            this->i++;
        }
        else { //Vector already full
            for(j=0; j<this->n_samples-1; j++){ //Left shift vector with new entrance
                this->samples[j]=this->samples[j+1];
            }
            this->samples[this->n_samples-1]=measure;
        }
        //Evaluate the median of vector samples*: [0], [1], ..., [nsamples-2], [nsamples-1]
        if(i<n_samples) return measure;
        else if(n_samples%2==0) return (this->samples[this->n_samples/2]+this->samples[this->n_samples/2-1])/2; //Even case
        else return this->samples[this->n_samples/2]; //Odd case
    }
    void free_mem(){
        free(this->samples);
    }
};

//ALGORITMO3: BESSEL IMA
class BesselIMA:public ALGO{
    private:
    unsigned int order;
    unsigned int cutoff;
    double *xv;
    double *yv;
    unsigned int i;
    const double GAIN = 1.626925725e+04;
    void allocateMemory(void){
        assert(this->order>0);
        this->xv=(double*) malloc(sizeof(double)*(this->order+1));
        assert(this->xv!=NULL);
        this->yv=(double*) malloc(sizeof(double)*(this->order+1));
        assert(this->yv!=NULL);
        this->i=0;
    }

    public:
    BesselIMA(){
        this->order=4;
        this->cutoff=5;
        this->allocateMemory();
    }
    unsigned long filter(unsigned long measure){
        if(this->i < this->order+1) { //Vectors not full
            xv[this->i]=measure / this->GAIN;
            yv[this->i]=measure / this->GAIN;
            this->i++;
            return measure;
        }
        else { //Vector already full
            xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4]; 
            xv[4] = measure / this->GAIN;
            yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; 
            yv[4] =   (xv[0] + xv[4]) + 4 * (xv[1] + xv[3]) + 6 * xv[2]
                + ( -0.5517688291 * yv[0]) + (  2.5441192827 * yv[1])
                + ( -4.4173936876 * yv[2]) + (  3.4240597840 * yv[3]);
            return long(yv[4]);
        }
    }
    void free_mem(){
        free(this->xv);
        free(this->yv);
    }
};

//ALGORITMO4: STABILITY RESEARCH
/*
Creazione di un filtro Bessel non solo immune ai disturbi ad alta frequenza,
ma anche immune a misure "statisticamente improbabili"
*/
class StabRes:public ALGO{
    private:
    unsigned int n_samples; //Numero elementi array tampone interno
    unsigned long *samples; //Array tampone interno
    unsigned int i; //Indice per il popolamento dell'array tampone interno
    unsigned int ecc;   //Eccezioni consecutive alle misure aspettate
    ALGO* internalFilter;
    void allocateMemory(void){
        assert(this->n_samples>0);
        this->samples=(unsigned long*) malloc(sizeof(unsigned long)*this->n_samples);
        assert(this->samples!=NULL);
        this->i=0;
    }
    unsigned long getStDev(void) {
        unsigned long sum = 0, mean, SD = 0;
        int i;
        for (i = 0; i < this->n_samples; ++i) {
            sum += this->samples[i];
        }
        mean = sum / this->n_samples;
        for (i = 0; i < this->n_samples; ++i) {
            SD += pow(this->samples[i] - mean, 2);
        }
        return sqrt(SD / 10);
    }
    unsigned long getAvg(void) {
        unsigned long sum = 0, mean;
        int l;
        for (l = 0; l < this->n_samples; l++) {
            sum += this->samples[l];
        }
        mean = sum / this->n_samples;
        return mean;
    }


    public:
    StabRes(){
        this->n_samples=1;
        this->allocateMemory();
    }
    StabRes(unsigned int n_samples, ALGO* internalFilter){
        this->n_samples=n_samples;
        this->allocateMemory();
        this->internalFilter=internalFilter;
        this->i=0;
        this->ecc=0;
    }
    unsigned long filter(unsigned long measure) {
        int j=0;
        if(this->i < this->n_samples) { //Vector not full: !critical point -> riempimento array tampone
            unsigned long filtered = this->internalFilter->filter(measure);
            samples[this->i]=filtered;
            this->i++;
            return measure;
        }
        else { //Vector already full
            const unsigned long stDev = this->getStDev();
            const unsigned long avg = this->getAvg();
            long diff = measure-avg;
            diff=abs(diff);
            //Se la lettura è statisticamente improbabile sia corretta, allora restituisco un valore fittizio
            if((diff>4*stDev)&&(this->ecc<this->n_samples)) {
                this->ecc++;
                return this->internalFilter->filter(avg);
            }
            //Lettura ripetuta fuori dal range va considerata corretta
            else if(this->ecc>=this->n_samples){
                this->ecc--;
                for(j=0; j<this->n_samples-1; j++){ //Left shift vector
                    this->samples[j]=this->samples[j+1];
                }
                this->samples[this->n_samples-1]=measure;
                return measure;
            }
            //Se la lettura è statisticamente probabile (oppure ripetitiva, quindi veritiera) allora la considero
            else{
                this->ecc=0;
                for(j=0; j<this->n_samples-1; j++){ //Left shift vector
                    this->samples[j]=this->samples[j+1];
                }
                this->samples[this->n_samples-1]=this->internalFilter->filter(measure);
                return this->samples[this->n_samples-1];
            }
        }
    }
    void free_mem(){
        free(this->samples);
    }
};

#endif
