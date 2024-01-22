#include "COND.hpp"


COND::COND(){
	this->calfact=1;
	this->offset=0;
    this->n_adcs=0;
    this->algo=NULL;
    this->mem=NULL;
}

void COND::init(HX711** adcs, unsigned int n_adcs) {
	this->adcs=adcs;
    this->n_adcs=n_adcs;
	//Initialize corrector factors (to balance via software gain factors differences)
	this->correction_factors = (unsigned int*) malloc(sizeof(unsigned int)*this->n_adcs);
	assert(this->correction_factors!=NULL);
	int i;
	unsigned int max_gain=32;
	for(i=0; i<this->n_adcs; i++)  if(this->adcs[i]->get_gain_value() > max_gain) max_gain=this->adcs[i]->get_gain_value();	//Find max gain
	for(i=0; i<this->n_adcs; i++) this->correction_factors[i]=max_gain/this->adcs[i]->get_gain_value();	//Balance all ADCs to this max gain
	//Save last measures
	this->last_read = (unsigned long*) malloc(sizeof(unsigned long)*this->n_adcs);
	assert(this->last_read!=NULL);
}

void COND::set_calfact(float calfact){
	this->calfact=calfact;
    if(this->mem!=NULL) this->mem->saveCalfact(this->get_calfact());
}

void COND::set_offset(long offset){
	this->offset=offset;
    if(this->mem!=NULL) this->mem->saveOffset(this->get_offset());
}

void COND::set_mem(SPIFFS* mem) {
	this->mem=mem;
	if(this->mem!=NULL) {
        this->calfact=this->mem->retrieveCalfact();
        this->offset=this->mem->retrieveOffset();
    }
}

long COND::get_offset(void){
	return this->offset;
}

float COND::get_calfact(void){
	return this->calfact;
}

unsigned long COND::read(void)
{
	unsigned long result=0;
    unsigned int read=0;
    unsigned int i=0;
    for(i=0;i<this->n_adcs;i++){
        read=this->adcs[i]->read() * this->correction_factors[i]; //<-----------------QUI
		this->last_read[i]=read;
        result+=read;   
    }
    return this->algo->filter(result);
}

unsigned long COND::read_average(unsigned int times) 
{
	unsigned long sum = 0;
	for (unsigned int i = 0; i < times; i++) 
	{
		sum += this->read();
	}
	return sum / times;
}

float COND::get_units(unsigned int times)
{
	long avg = this->read_average(times);
	avg=avg-this->offset; 
	
	return avg / this->calfact;
}

long COND::tare(unsigned int times) 
{
	unsigned long avg = 0; 
	avg = this->read_average(times);
	this->set_offset(avg);
	
	return this->get_offset();
}

float COND::calib(int weight, unsigned int times)
{
	if(weight==0) return this->calfact;
	long avg=0;
	avg = this->read_average(times);
	float calfact = (1.0*(avg - this->offset)) / (1.0*weight);
	if(calfact!=0) this->set_calfact(calfact);

	return this->get_calfact();
}

unsigned long COND::get_last_read(unsigned int n_adc){
	if(n_adc>=this->n_adcs) return 0;
	return this->last_read[n_adc];
}

void COND::free_mem(void){
	free(this->last_read);
	free(this->correction_factors);
}