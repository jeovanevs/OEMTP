/**
 *  Copyright 2015 by Renato Monaro and Silvio Giuseppe
 *  Copyright 2016 by Renato Monaro, Silvio Giuseppe and Heitor Kenzo Koga
 *  Copyright 2017 by Renato Monaro, Silvio Giuseppe and Heitor Kenzo Koga
 *
 * This file is part of Open Electromagnetic Transient Program - OEMTP.
 * 
 * OEMTP is free software: you can redistribute 
 * it and/or modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation, either 
 * version 3 of the License, or (at your option) any later version.
 * 
 * Some open source application is distributed in the hope that it will 
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

#include "components.h"

using namespace std;
using namespace oemtp;

Component::Component(){
	};
bool Component::Reset(){
	gsl_vector_set_zero(Ic);
	gsl_vector_set_zero(&View_V_Pri.vector);
	gsl_vector_set_zero(&View_I_Hist_Pri.vector);
	return true;
	}

Component::~Component(){
	gsl_vector_free(Ic);
	gsl_matrix_free(Reff);
	gsl_matrix_free(Refflu);
	gsl_permutation_free(p);
	};
	
bool Component::Compute_Ih(bool e){};

unsigned Component::Get_N_Branches(){
	return Reff->size1;
	}

unsigned Component::Get_N_Nodes(){
	return Alias.size();
	}
	
	
bool Component::Compute_I(){
	gsl_blas_dgemv (CblasNoTrans, 1.0,&View_Gpr.matrix, &View_V_Pri.vector, 0.0, Ic); //I'=Gpr*V
	gsl_vector_add(Ic,&View_I_Hist_Pri.vector); //I=I+I_h   
	return true;
	}
	
bool Component::Set_Views(gsl_matrix_view GprV,gsl_vector_view Vv,gsl_vector_view IhV){
	View_Gpr=GprV;
	View_V_Pri=Vv;
	View_I_Hist_Pri=IhV;
  	Refflu=gsl_matrix_alloc(Reff->size1,Reff->size2);
  	p = gsl_permutation_alloc (Reff->size1);
	return true;
	}
	
string Component::Get_Alias(unsigned Al){
	return Alias[Al];			
	}
	
double Component::Get_I(unsigned N){
	if(N<Ic->size)
		return gsl_vector_get(Ic,N);
	return GSL_NAN;
	}
	
double Component::Get_V(unsigned N){
	if(N<View_V_Pri.vector.size)
		return gsl_vector_get(&View_V_Pri.vector,N);
	return GSL_NAN;
	}

bool Component::Compute_Gpr(){


	gsl_matrix_memcpy (Refflu, Reff);
	int s;

	gsl_linalg_LU_decomp (Refflu, p, &s);
    gsl_linalg_LU_invert (Refflu, p,&View_Gpr.matrix);	

		#ifdef DEBUG
	    for(unsigned k=0;k<View_Gpr.matrix.size1;k++){
        for(unsigned j=0;j<View_Gpr.matrix.size2;j++){	
        	cout<<gsl_matrix_get(&View_Gpr.matrix,k,j)<<" ";
	    }
	    cout<<endl;
	    }
	   #endif
	return true;
	}

bool Component::G_Changed(){
	bool ret;
	ret=Changed;
	Changed=false;
	return ret;
	}
	
bool Component::Set_Value(unsigned k, unsigned l,double value){
	if((k<Reff->size1)&&(l<Reff->size2)){
		gsl_matrix_set(Reff,k,l,value);
		Changed=true;
		return true;
		}
	return false;
	}
			
Resistor::Resistor(string N1,string N2,double Res){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	gsl_matrix_set(Reff,0,0,Res);
	Changed=true;
	}
	
Resistor::Resistor(vector<string> N,vector<vector<double> > Res){
	for(unsigned k=0;k<N.size();k++)
		Alias.push_back(N[k]);
	Reff=gsl_matrix_alloc(Res.size(),Res.size());
	Ic=gsl_vector_alloc(Res.size());
	
	for(unsigned k=0;k<Res.size();k++)
		for(unsigned l=0;l<Res.size();l++)
			gsl_matrix_set(Reff,k,l,Res[k][l]);
	Changed=true;
	}
	
bool Resistor::Compute_Ih(bool e){
	return true;
	}
	
bool Resistor::Set_Value(double value){
	gsl_matrix_set(Reff,0,0,value);
	Changed=true;
	return true;
	}

Switch::Switch(string N1,string N2,bool IniStatus,double ResON, double ResOFF){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	Status=IniStatus;
	Ron=ResON;
	Roff=ResOFF;
	if(Status)
		gsl_matrix_set(Reff,0,0,Ron);
	else
		gsl_matrix_set(Reff,0,0,Roff);
	Changed=true;
	}
	
Switch::Switch(string N1,string N2,bool IniStatus){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	Status=IniStatus;
	Ron=R_MIN;
	Roff=R_MAX;
	if(Status)
		gsl_matrix_set(Reff,0,0,Ron);
	else
		gsl_matrix_set(Reff,0,0,Roff);
	Changed=true;
	}
	
Switch::Switch(string N1,string N2){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	Status=false;
	Ron=R_MIN;
	Roff=R_MAX;
	if(Status)
		gsl_matrix_set(Reff,0,0,Ron);
	else
		gsl_matrix_set(Reff,0,0,Roff);
	Changed=true;
	}
	
void Switch::Open(){
	if(Status==true)
		Changed=true;
	else
		Changed=false;
	Status=false;
	gsl_matrix_set(Reff,0,0,Roff);
	}
	
void Switch::Close(){
	if(Status==false)
		Changed=true;
	else
		Changed=false;
	Status=true;
	gsl_matrix_set(Reff,0,0,Ron);
	}	
	
bool Switch::Get_Status(){
	return Status;
	}

void Switch::Set_Status(bool s){
	if(Status==s)
		Changed=false;
	else
		Changed=true;
	if(s)
		gsl_matrix_set(Reff,0,0,Ron);
	else
		gsl_matrix_set(Reff,0,0,Roff);	
	Status=s;
	}
	
bool Switch::Set_Ron(double value){
	if((value<=R_MAX)&&(value>=R_MIN)){
		Ron=value;
		return true;
		}
	return false;
	}
	
bool Switch::Set_Roff(double value){
	if((value<=R_MAX)&&(value>=R_MIN)){
		Roff=value;
		return true;
		}
	return false;
	}
	
Switch2::Switch2(string N1,string N2,bool IniStatus,double ResON, double ResOFF){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	Status=IniStatus;
	Ron=ResON;
	Roff=ResOFF;
	if(Status)
		gsl_matrix_set(Reff,0,0,Ron);
	else
		gsl_matrix_set(Reff,0,0,Roff);
	Changed=true;
	}


bool Switch2::Compute_Ih(bool e){
	if(!Status){
		double R=gsl_matrix_get(Reff,0,0);
		if(R<Roff){
			R*=3;
			gsl_matrix_set(Reff,0,0,R);
			}			
		}
	return true;	
	}
	
bool Diode::Compute_Ih(bool e){
	if(Get_V(0)<=0.0){
		Open();
		}
	if(Get_V(0)>0.7){
		Close();
		}
	return true;	
	}
	
Voltmeter::Voltmeter(string N1,string N2){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	gsl_matrix_set(Reff,0,0,R_MAX);
	Changed=true;
	}
Voltmeter::Voltmeter(vector<string> N){
	for(unsigned k=0;k<N.size();k++)
		Alias.push_back(N[k]);
	Reff=gsl_matrix_alloc(Alias.size()/2,Alias.size()/2);
	Ic=gsl_vector_alloc(Alias.size()/2);
	for(unsigned k=0;k<Alias.size()/2;k++)
		gsl_matrix_set(Reff,k,k,R_MAX);
	Changed=true;
	}
Ammeter::Ammeter(string N1,string N2){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	gsl_matrix_set(Reff,0,0,R_MIN);
	Changed=true;
	}
Ammeter::Ammeter(vector<string> N){
	for(unsigned k=0;k<N.size();k++)
		Alias.push_back(N[k]);
	Reff=gsl_matrix_alloc(Alias.size()/2,Alias.size()/2);
	Ic=gsl_vector_alloc(Alias.size()/2);
	for(unsigned k=0;k<Alias.size()/2;k++)
		gsl_matrix_set(Reff,k,k,R_MIN);
	Changed=true;
	}
	
	
Current_Source::Current_Source(string N1,string N2,double Is, double Res){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	gsl_matrix_set(Reff,0,0,Res);
	In=Is;
	Changed=true;
	}
		
bool Current_Source::Compute_Ih(bool e){
	gsl_vector_set(&View_I_Hist_Pri.vector,0,-In);
	return true;
	}
	
DC_Source::DC_Source(string N1,string N2,double Volt, double Res){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	gsl_matrix_set(Reff,0,0,Res);
	In=Volt/Res;
	Changed=true;
	}
	
AC_Source::AC_Source(string N1,string N2,double Volt, double Freq, double Ang,double Res, double Dt){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	gsl_matrix_set(Reff,0,0,Res);
	In=(Volt*sqrt(2))/Res;
	Frequency=Freq;
	Phase=M_PI*Ang/180.0;
	dt=Dt;
	T=0;
	Changed=true;
	}
bool AC_Source::Compute_Ih(bool e){
	gsl_vector_set(&View_I_Hist_Pri.vector,0,-In*sin(2*M_PI*Frequency*T+Phase));
	T+=dt;
	return true;
	}

void AC_Source::Set_Frequency(double F){
	Frequency=F;
	}
void AC_Source::Set_Voltage(double V){
	In=(V*sqrt(2))/gsl_matrix_get(Reff,0,0);
	}
	
void AC_Source::Set_Angle(double P){
	Phase=P;
	}
			
Inductor::Inductor(string N1,string N2, double L, double dT){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	gsl_matrix_set(Reff,0,0,2*L/dT);
	dt=dT;
	Changed=true;
	}
	
bool Inductor::Compute_Ih(bool e){
	if(e){
		gsl_vector_memcpy(&View_I_Hist_Pri.vector,Ic);
		}
	else{
		gsl_blas_dgemv (CblasNoTrans, 1.0, &View_Gpr.matrix, &View_V_Pri.vector, 0.0, &View_I_Hist_Pri.vector);
		gsl_vector_add(&View_I_Hist_Pri.vector,Ic); //I=I'+I_h
		}
	return true;
	}
	
bool Inductor::Set_Value(double value){
	gsl_matrix_set(Reff,0,0,2*value/dt);
	Changed=true;
	return true;
	}
	
Capacitor::Capacitor(string N1,string N2, double C, double dT){
	Alias.push_back(N1);
	Alias.push_back(N2);
	Reff=gsl_matrix_alloc(1,1);
	Ic=gsl_vector_alloc(1);
	gsl_matrix_set(Reff,0,0,dT/(2*C));
	Changed=true;
	dt=dT;
	}
bool Capacitor::Compute_Ih(bool e){
	gsl_blas_dgemv (CblasNoTrans, -1.0, &View_Gpr.matrix, &View_V_Pri.vector, 0.0, &View_I_Hist_Pri.vector);
	if(!e){
		gsl_vector_sub(&View_I_Hist_Pri.vector,Ic); //I=-I'-I_h
		}
	return true;
	}
	
bool Capacitor::Set_Value(double value){
		gsl_matrix_set(Reff,0,0,dt/(2*value));
	Changed=true;
	return true;
	}	

	
Lossless_Line::Lossless_Line(string N1,string N2,double d, double l, double c,double dT){
	Alias.push_back(N1);
	Alias.push_back("TERRA");
	Alias.push_back(N2);
	Alias.push_back("TERRA");
	Zc=sqrt(l/c);
	Tau=d*sqrt(l*c);
	dt=dT;
	Reff=gsl_matrix_alloc(2,2);
	Ic=gsl_vector_alloc(2);
	gsl_matrix_set(Reff,0,0,Zc);
	gsl_matrix_set(Reff,1,1,Zc);
	k_eff=Tau/dT;
	k_inf=floor(k_eff);
	k_sup=ceil(k_eff);
	Vm=gsl_vector_alloc(k_sup+1);
	Vk=gsl_vector_alloc(k_sup+1);
	Imk=gsl_vector_alloc(k_sup+1);
	Ikm=gsl_vector_alloc(k_sup+1);
	Changed=true;	

	//cout<<"LossLessLine Zc:"<<Zc<<" "<<"Tau:"<<Tau<<" "<<"Keff:"<<k_eff<<" "<<"Ks:"<<k_sup<<" "<<"Ki:"<<k_inf<<" "<<endl;
	
	}
	
Lossless_Line::Lossless_Line(string N1,string N2,double d, double xl, double yc, double f,double dT){
	Alias.push_back(N1);
	Alias.push_back("TERRA");
	Alias.push_back(N2);
	Alias.push_back("TERRA");
	Zc=sqrt(xl/yc);
	Tau=d*sqrt(xl*yc)/(2*M_PI*f);
	dt=dT;
	Reff=gsl_matrix_alloc(2,2);
	Ic=gsl_vector_alloc(2);
	gsl_matrix_set(Reff,0,0,Zc);
	gsl_matrix_set(Reff,1,1,Zc);
	k_eff=Tau/dT;
	k_inf=floor(k_eff);
	k_sup=ceil(k_eff);
	Vm=gsl_vector_alloc(k_sup);
	Vk=gsl_vector_alloc(k_sup);
	Imk=gsl_vector_alloc(k_sup);
	Ikm=gsl_vector_alloc(k_sup);
	alpha=(k_inf-k_eff);
	Changed=true;

		//cout<<"LossLessLine Zc:"<<Zc<<" "<<"Tau:"<<Tau<<" "<<"Keff:"<<k_eff<<" "<<"Ks:"<<k_sup<<" "<<"Ki:"<<k_inf<<" "<<endl;
	}
	
Lossless_Line::Lossless_Line(string N1,string N2, double Z, double T, double dT){
	Alias.push_back(N1);
	Alias.push_back("TERRA");
	Alias.push_back(N2);
	Alias.push_back("TERRA");
	Zc=Z;
	Tau=T;
	dt=dT;
	Reff=gsl_matrix_alloc(2,2);
	Ic=gsl_vector_alloc(2);
	gsl_matrix_set(Reff,0,0,Zc);
	gsl_matrix_set(Reff,1,1,Zc);
	k_eff=Tau/dT;
	k_inf=floor(k_eff);
	k_sup=ceil(k_eff);
	Vm=gsl_vector_alloc(k_sup);
	Vk=gsl_vector_alloc(k_sup);
	Imk=gsl_vector_alloc(k_sup);
	Ikm=gsl_vector_alloc(k_sup);
	alpha=(k_inf-k_eff);
	Changed=true;
	}
	
bool Lossless_Line::Compute_Ih(bool e){
	//cout<<"LossLessLine Zc:"<<Zc<<" "<<"Tau:"<<Tau<<" "<<"Keff:"<<k_eff<<" "<<"Ks:"<<k_sup<<" "<<"Ki:"<<k_inf<<" "<<endl;
	for(unsigned k=k_sup;k>0;k--){
		gsl_vector_set(Vk,k,gsl_vector_get(Vk,k-1));
		gsl_vector_set(Vm,k,gsl_vector_get(Vm,k-1));
		gsl_vector_set(Imk,k,gsl_vector_get(Imk,k-1));
		gsl_vector_set(Ikm,k,gsl_vector_get(Ikm,k-1));
		}
	gsl_vector_set(Ikm,0,gsl_vector_get(Ic,0));
	gsl_vector_set(Imk,0,gsl_vector_get(Ic,1));
	gsl_vector_set(Vk,0,gsl_vector_get(&View_V_Pri.vector,0));
	gsl_vector_set(Vm,0,gsl_vector_get(&View_V_Pri.vector,1));
	
	Vk_tau=alpha*gsl_vector_get(Vk,k_sup)+(1-alpha)*gsl_vector_get(Vk,k_inf);
	Vm_tau=alpha*gsl_vector_get(Vm,k_sup)+(1-alpha)*gsl_vector_get(Vm,k_inf);
	Ikm_tau=alpha*gsl_vector_get(Ikm,k_sup)+(1-alpha)*gsl_vector_get(Ikm,k_inf);
	Imk_tau=alpha*gsl_vector_get(Imk,k_sup)+(1-alpha)*gsl_vector_get(Imk,k_inf);
	
	/*Vk_tau=gsl_vector_get(Vk,k_inf);
	Vm_tau=gsl_vector_get(Vm,k_inf);
	Ikm_tau=gsl_vector_get(Ikm,k_inf);
	Imk_tau=gsl_vector_get(Imk,k_inf);*/

	gsl_vector_set(&View_I_Hist_Pri.vector,0,-Vm_tau/Zc-Imk_tau);
	gsl_vector_set(&View_I_Hist_Pri.vector,1,-Vk_tau/Zc-Ikm_tau);
	Changed=true;
	return true;
	}


bool Lossless_Line::Set_Value(double p){
	if(p<0.0)
		return false;
	k_eff=p*Tau/dt;
	k_inf=floor(k_eff);
	k_sup=ceil(k_eff);
	return true;
	}
	
bool Lossless_Line::Reset(){
	gsl_vector_set_zero(Ic);
	gsl_vector_set_zero(&View_V_Pri.vector);
	gsl_vector_set_zero(&View_I_Hist_Pri.vector);
	gsl_vector_set_zero(Vm);
	gsl_vector_set_zero(Vk);
	gsl_vector_set_zero(Ikm);
	gsl_vector_set_zero(Imk);
	return true;
	}
	

Line::Line(string N1,string N2,double d, double l, double c, double r,double dT){
	Alias.push_back(N1);
	Alias.push_back("TERRA");
	Alias.push_back(N2);
	Alias.push_back("TERRA");
	dt=dT;
	Zc=sqrt(l/c);
	Tau=d*sqrt(l*c);
	Reff=gsl_matrix_alloc(2,2);
	Ic=gsl_vector_alloc(2);
	R=r*d;
	gsl_matrix_set(Reff,0,0,Zc+R/4);
	gsl_matrix_set(Reff,1,1,Zc+R/4);
	k_eff=Tau/dt;
	k_inf=floor(k_eff);
	k_sup=ceil(k_eff);
	Vm=gsl_vector_alloc(k_sup+1);
	Vk=gsl_vector_alloc(k_sup+1);
	Imk=gsl_vector_alloc(k_sup+1);
	Ikm=gsl_vector_alloc(k_sup+1);
	Changed=true;
	//cout<<"Line Zc:"<<Zc<<" "<<"Tau:"<<Tau<<" "<<"Keff:"<<k_eff<<" "<<"Ks:"<<k_sup<<" "<<"Ki:"<<k_inf<<" "<<endl;
	}
	
Line::Line(string N1,string N2,double d, double xl, double yc, double f, double r,double dT){
	Alias.push_back(N1);
	Alias.push_back("TERRA");
	Alias.push_back(N2);
	Alias.push_back("TERRA");
	dt=dT;
	Zc=sqrt(xl/yc);
	Tau=d*sqrt(xl*yc)/(2*M_PI*f);
	Reff=gsl_matrix_alloc(2,2);
	Ic=gsl_vector_alloc(2);
	R=r*d;
	gsl_matrix_set(Reff,0,0,Zc+R/4);
	gsl_matrix_set(Reff,1,1,Zc+R/4);
	k_eff=Tau/dT;
	k_inf=floor(k_eff);
	k_sup=ceil(k_eff);
	Vm=gsl_vector_alloc(k_sup+1);
	Vk=gsl_vector_alloc(k_sup+1);
	Imk=gsl_vector_alloc(k_sup+1);
	Ikm=gsl_vector_alloc(k_sup+1);
	alpha=(k_inf-k_eff);
	Changed=true;

		//cout<<"Line Zc:"<<Zc<<" "<<"Tau:"<<Tau<<" "<<"Keff:"<<k_eff<<" "<<"Ks:"<<k_sup<<" "<<"Ki:"<<k_inf<<" "<<endl;
	}
	
Line::Line(string N1,string N2, double Z, double T, double r, double dT){
	Alias.push_back(N1);
	Alias.push_back("TERRA");
	Alias.push_back(N2);
	Alias.push_back("TERRA");
	dt=dT;
	Zc=Z;
	Tau=T;
	Reff=gsl_matrix_alloc(2,2);
	Ic=gsl_vector_alloc(2);
	R=r;
	gsl_matrix_set(Reff,0,0,Zc+R/4);
	gsl_matrix_set(Reff,1,1,Zc+R/4);
	k_eff=Tau/dT;
	k_inf=floor(k_eff);
	k_sup=ceil(k_eff);
	Vm=gsl_vector_alloc(k_sup+1);
	Vk=gsl_vector_alloc(k_sup+1);
	Imk=gsl_vector_alloc(k_sup+1);
	Ikm=gsl_vector_alloc(k_sup+1);
	alpha=(k_sup-k_eff);

	Changed=true;
	}
	
bool Line::Compute_Ih(bool e){
	//cout<<"Line Zc:"<<Zc<<" "<<"Tau:"<<Tau<<" "<<"R:"<<R<<" "<<"Keff:"<<k_eff<<" "<<"Ks:"<<k_sup<<" "<<"Ki:"<<k_inf<<" "<<endl;
	for(unsigned k=k_sup;k>0;k--){
		gsl_vector_set(Vk,k,gsl_vector_get(Vk,k-1));
		gsl_vector_set(Vm,k,gsl_vector_get(Vm,k-1));
		gsl_vector_set(Imk,k,gsl_vector_get(Imk,k-1));
		gsl_vector_set(Ikm,k,gsl_vector_get(Ikm,k-1));
		}
	gsl_vector_set(Ikm,0,gsl_vector_get(Ic,0));
	gsl_vector_set(Imk,0,gsl_vector_get(Ic,1));
	gsl_vector_set(Vk,0,gsl_vector_get(&View_V_Pri.vector,0));
	gsl_vector_set(Vm,0,gsl_vector_get(&View_V_Pri.vector,1));
	
	Vk_tau=alpha*gsl_vector_get(Vk,k_sup)+(1-alpha)*gsl_vector_get(Vk,k_inf);
	Vm_tau=alpha*gsl_vector_get(Vm,k_sup)+(1-alpha)*gsl_vector_get(Vm,k_inf);
	Ikm_tau=alpha*gsl_vector_get(Ikm,k_sup)+(1-alpha)*gsl_vector_get(Ikm,k_inf);
	Imk_tau=alpha*gsl_vector_get(Imk,k_sup)+(1-alpha)*gsl_vector_get(Imk,k_inf);
	
	/*Vk_tau=gsl_vector_get(Vk,k_sup);
	Vm_tau=gsl_vector_get(Vm,k_sup);
	Ikm_tau=gsl_vector_get(Ikm,k_sup);
	Imk_tau=gsl_vector_get(Imk,k_sup);*/

	gsl_vector_set(&View_I_Hist_Pri.vector,0,-(Zc/pow(Zc+R/4,2))*(Vm_tau+(Zc-R/4)*Imk_tau)-((R/4)/pow(Zc+R/4,2))*(Vk_tau+(Zc-R/4)*Ikm_tau));
	
	gsl_vector_set(&View_I_Hist_Pri.vector,1,-(Zc/pow(Zc+R/4,2))*(Vk_tau+(Zc-R/4)*Ikm_tau)-((R/4)/pow(Zc+R/4,2))*(Vm_tau+(Zc-R/4)*Imk_tau));
	
	return true;
	}
	
bool Line::Set_Value(double p){
	if(p<0.0)
		return false;
	k_eff=p*Tau/dt;
	k_inf=floor(k_eff);
	k_sup=ceil(k_eff);
	return true;
	}
	
bool Line::Reset(){
	gsl_vector_set_zero(Ic);
	gsl_vector_set_zero(&View_V_Pri.vector);
	gsl_vector_set_zero(&View_I_Hist_Pri.vector);
	gsl_vector_set_zero(Vm);
	gsl_vector_set_zero(Vk);
	gsl_vector_set_zero(Ikm);
	gsl_vector_set_zero(Imk);
	return true;
	}
	
//MLine::MLine(CableSet *C, vector<string> N, double l, double dT){
//	nModes=C->Get_N_Modes();
//	if((N.size()/2)!=nModes){
//		cout<<"Error Number of Nodes different of Number of Conductors"<<endl;
//		exit(0);
//		}
//		
//	V_Mode=gsl_vector_alloc(2*nModes);
//	I_Mode=gsl_vector_alloc(2*nModes);
//	Ih_Mode=gsl_vector_alloc(2*nModes);
//	Reff=gsl_matrix_alloc(2*nModes,2*nModes);
//	Ic=gsl_vector_alloc(2*nModes);
//	Zc_Mod=C->Get_Zc_Mode();
//	Zc_Phase=C->Get_Zc_Phase();
//	R_Mod=C->Get_R_Mode();
//	R_Phase=C->Get_R_Phase();
//	
//	Reff_Send=gsl_matrix_submatrix(Reff, 0, 0, nModes, nModes);
//	Reff_Recv=gsl_matrix_submatrix(Reff, nModes, nModes, nModes, nModes);
//	
//	gsl_matrix_add(&Reff_Send.matrix,Zc_Phase);
//	gsl_matrix_add(&Reff_Send.matrix,R_Phase);

//	gsl_matrix_add(&Reff_Recv.matrix,Zc_Phase);
//	gsl_matrix_add(&Reff_Recv.matrix,R_Phase);
//	
//	unsigned k_sup_max=0;
//	for(unsigned k=0;k<N.size();k+=2){
//		Alias.push_back(N[k]);
//		Alias.push_back("TERRA");
//		Alias.push_back(N[k+1]);
//		Alias.push_back("TERRA");
//		Zc_Mode.push_back(C->Get_Zc(k/2));
//		Tau_Mode.push_back(l/C->Get_Speed(k/2));
//		R_Mode.push_back(l*C->Get_Rm(k/2));

//		k_eff_Mode.push_back(Tau_Mode.back()/dT);
//		k_inf_Mode.push_back(floor(k_eff_Mode.back()));
//		k_sup_Mode.push_back(ceil(k_eff_Mode.back()));
//		alpha_Mode.push_back(k_inf_Mode.back()-k_eff_Mode.back());
//		if(k_sup_Mode.back()>k_sup_max)
//			k_sup_max=k_sup_Mode.back();
//		}
//	Vm_Mode=gsl_matrix_alloc(k_sup_max+1,nModes);
//	Vk_Mode=gsl_matrix_alloc(k_sup_max+1,nModes);
//	Ikm_Mode=gsl_matrix_alloc(k_sup_max+1,nModes);
//	Imk_Mode=gsl_matrix_alloc(k_sup_max+1,nModes);
//		
//	Ti=gsl_matrix_alloc(2*nModes,2*nModes);
//	Tv_inv=gsl_matrix_alloc(2*nModes,2*nModes);
//	
//	gsl_matrix_view Ti_send,Ti_recv,Tv_inv_send,Tv_inv_recv;
//	Ti_send=gsl_matrix_submatrix(Ti,0, 0, nModes,nModes);
//	Ti_recv=gsl_matrix_submatrix(Ti, nModes, nModes, nModes,nModes);
//	Tv_inv_send=gsl_matrix_submatrix(Tv_inv,0, 0, nModes,nModes);
//	Tv_inv_recv=gsl_matrix_submatrix(Tv_inv, nModes, nModes, nModes,nModes);
//	gsl_matrix_memcpy(&Ti_send.matrix,C->Get_Ti());
//	gsl_matrix_memcpy(&Ti_recv.matrix,C->Get_Ti());
//	gsl_matrix_memcpy(&Tv_inv_send.matrix,C->Get_Tv_Inverse());
//	gsl_matrix_memcpy(&Tv_inv_recv.matrix,C->Get_Tv_Inverse());
//	Changed=true;
//	}
//	
//	
//bool MLine::Compute_Ih(bool e){
////	gsl_blas_dgemv(CblasNoTrans, 1.0,Tv_inv, &View_V_Pri.vector, 0.0, V_Mode); //Vm=Tv^-1 Vf
//	gsl_vector_memcpy(V_Mode,&View_V_Pri.vector);
//	gsl_vector_memcpy(I_Mode,Ic);
//	for(unsigned m=0;m<nModes;m++){
//		Vk=gsl_matrix_column(Vk_Mode,m);
//		Vm=gsl_matrix_column(Vm_Mode,m);
//		Ikm=gsl_matrix_column(Ikm_Mode,m);
//		Imk=gsl_matrix_column(Imk_Mode,m);
//		alpha=alpha_Mode[m];
//		k_inf=k_inf_Mode[m];
//		k_sup=k_sup_Mode[m];
//		Zc=Zc_Mode[m];
//		R=R_Mode[m];
//		for(unsigned k=k_sup;k>0;k--){
//			gsl_vector_set(&Vk.vector,k,gsl_vector_get(&Vk.vector,k-1));
//			gsl_vector_set(&Vm.vector,k,gsl_vector_get(&Vm.vector,k-1));
//			gsl_vector_set(&Imk.vector,k,gsl_vector_get(&Imk.vector,k-1));
//			gsl_vector_set(&Ikm.vector,k,gsl_vector_get(&Ikm.vector,k-1));
//			}
//		gsl_vector_set(&Ikm.vector,0,gsl_vector_get(I_Mode,2*m));
//		gsl_vector_set(&Imk.vector,0,gsl_vector_get(I_Mode,2*m+1));
//		gsl_vector_set(&Vk.vector,0,gsl_vector_get(V_Mode,2*m));
//		gsl_vector_set(&Vm.vector,0,gsl_vector_get(V_Mode,2*m+1));
//	
//		Vk_tau=alpha*gsl_vector_get(&Vk.vector,k_sup)+(1-alpha)*gsl_vector_get(&Vk.vector,k_inf);
//		Vm_tau=alpha*gsl_vector_get(&Vm.vector,k_sup)+(1-alpha)*gsl_vector_get(&Vm.vector,k_inf);
//		Ikm_tau=alpha*gsl_vector_get(&Ikm.vector,k_sup)+(1-alpha)*gsl_vector_get(&Ikm.vector,k_inf);
//		Imk_tau=alpha*gsl_vector_get(&Imk.vector,k_sup)+(1-alpha)*gsl_vector_get(&Imk.vector,k_inf);
//	
//		/*Vk_tau=gsl_vector_get(&Vk.vector,k_inf);
//		Vm_tau=gsl_vector_get(Vm,k_inf);
//		Ikm_tau=gsl_vector_get(Ikm,k_inf);
//		Imk_tau=gsl_vector_get(Imk,k_inf);*/

//		gsl_vector_set(Ih_Mode,2*m,-(Zc/pow(Zc+R/4,2))*(Vm_tau+(Zc-R/4)*Imk_tau)-((R/4)/pow(Zc+R/4,2))*(Vk_tau+(Zc-R/4)*Ikm_tau));
//		gsl_vector_set(Ih_Mode,2*m+1,-(Zc/pow(Zc+R/4,2))*(Vk_tau+(Zc-R/4)*Ikm_tau)-((R/4)/pow(Zc+R/4,2))*(Vm_tau+(Zc-R/4)*Imk_tau));
//		}
//	//gsl_blas_dgemv(CblasNoTrans, 1.0,Ti,Ih_Mode, 0.0, &View_I_Hist_Pri.vector); //Ihf=Ti*Ihm
//	gsl_vector_memcpy(&View_I_Hist_Pri.vector,Ih_Mode);
//	return true;
//	}
	
/*	
MLine2::MLine2(CableSet *C, vector<string> N, double l, double dT){
	nModes=C->Get_N_Modes();
	if((N.size()/2)!=nModes){
		cout<<"Error Number of Nodes different of Number of Conductors"<<endl;
		exit(0);
		}
	Reff=gsl_matrix_alloc(2*nModes,2*nModes);
	gsl_matrix_set_zero(Reff);
	Ic=gsl_vector_alloc(2*nModes);
	Zc_Phase=C->Get_Zc_Phase();
	R_Phase=C->Get_R_Phase();
	
	//gsl_matrix_scale(R_Phase,1.0/4.0);
	
	
	Gpr_MODE=gsl_matrix_alloc(2*nModes,2*nModes);
	V_Pri_MODE=gsl_vector_alloc(2*nModes);
	I_Hist_Pri_MODE=gsl_vector_alloc(2*nModes);
	
	gsl_matrix_view Reff_Send,Reff_Recv;
	
	Reff_Send=gsl_matrix_submatrix(Reff, 0, 0, nModes, nModes);
	Reff_Recv=gsl_matrix_submatrix(Reff, nModes, nModes, nModes, nModes);
	
	gsl_matrix_add(&Reff_Send.matrix,Zc_Phase);
	gsl_matrix_add(&Reff_Send.matrix,R_Phase);

	gsl_matrix_add(&Reff_Recv.matrix,Zc_Phase);
	gsl_matrix_add(&Reff_Recv.matrix,R_Phase);
	for(unsigned k=0;k<nModes;k++){
		Lines.push_back(new Line(N[2*k],N[2*k+1], C->Get_Zc_Mode(k), l/C->Get_Speed(k), l*C->Get_R_Mode(k), dT));
		Alias.push_back(N[k]);
		Alias.push_back("TERRA");
		Alias.push_back(N[2*k+1]);
		Alias.push_back("TERRA");
		}
	for(unsigned k=0;k<nModes;k++){
		Lines[k]->Set_Views(gsl_matrix_submatrix(Gpr_MODE, 2*k, 2*k, 2,2),gsl_vector_subvector(V_Pri_MODE,2*k,2),gsl_vector_subvector(I_Hist_Pri_MODE,2*k,2));
		Lines[k]->Compute_Gpr();
		}
		
	Ti=gsl_matrix_alloc(2*nModes,2*nModes);
	Tv_inv=gsl_matrix_alloc(2*nModes,2*nModes);
	
	gsl_matrix_view Ti_send,Ti_recv,Tv_inv_send,Tv_inv_recv;
	Ti_send=gsl_matrix_submatrix(Ti,0, 0, nModes,nModes);
	Ti_recv=gsl_matrix_submatrix(Ti, nModes, nModes, nModes,nModes);
	Tv_inv_send=gsl_matrix_submatrix(Tv_inv,0, 0, nModes,nModes);
	Tv_inv_recv=gsl_matrix_submatrix(Tv_inv, nModes, nModes, nModes,nModes);
	gsl_matrix_memcpy(&Ti_send.matrix,C->Get_Ti());
	gsl_matrix_memcpy(&Ti_recv.matrix,C->Get_Ti());
	gsl_matrix_memcpy(&Tv_inv_send.matrix,C->Get_Tv_Inverse());
	gsl_matrix_memcpy(&Tv_inv_recv.matrix,C->Get_Tv_Inverse());
	Changed=true;
	}
	
bool MLine2::Compute_Ih(bool e){
	gsl_blas_dgemv(CblasNoTrans, 1.0,Tv_inv, &View_V_Pri.vector, 0.0, V_Pri_MODE); 	//Transforma Vfase-Vmode ----- equivalente ao compute V
	for(unsigned k=0;k<nModes;k++){
		Lines[k]->Compute_I();
		Lines[k]->Compute_Ih(e);
		}
	gsl_blas_dgemv(CblasNoTrans, 1.0,Ti,I_Hist_Pri_MODE, 0.0, &View_I_Hist_Pri.vector); //Transforma Ihmodo-Ihfase ----- equivalente ao compute Ih
	}
	
bool MLine2::Reset(){	
	for(unsigned k=0;k<nModes;k++){
		Lines[k]->Reset();
		}
	return true;
	}
*/
InductionMachine::InductionMachine(string N1, string N2, string N3, double r1, double r2, double l1, double l2, double lh, double jj, double kd, int p, double mt, double Dt) {
	
	Alias.push_back(N1);
	Alias.push_back("TERRA");
	Alias.push_back(N2);
	Alias.push_back("TERRA");
	Alias.push_back(N3);
	Alias.push_back("TERRA");

	Reff = gsl_matrix_alloc(3,3);
	Ic = gsl_vector_alloc(3);

	data.R1 = r1;
	data.R2 = r2;
	data.L1 = l1;
	data.L2 = l2;
	data.LH = lh;
	data.sig = 1 -((lh*lh)/(l1*l2));
	data.JJ = jj;
	data.KD = kd;
	data.P = p;
	data.MT = mt;
	data.dt = Dt;

	T = 0.0;
	
	this->y[0] = 0.0; this->y[1] = 0.0; this->y[2] = 0.0; this->y[3] = 0.0;
	this->y[4] = 0.0; this->y[5] = 0.0; this->y[6] = 0.0;

	gsl_matrix_set(Reff,0,0, 2*lh/dt); /*Ref: Power Systems Eletromagnetic Transients Simulation pg 185-186*/
	gsl_matrix_set(Reff,1,1, 2*lh/dt);
	gsl_matrix_set(Reff,2,2, 2*lh/dt);

	Changed=true;
}


int InductionMachine::func (double t, const double Y[], double F[], void *params) {
	struct IM_ODE_Data *Param = (struct IM_ODE_Data *)params;
	int z;
	double T1, T2, sig;
	double w2, md, a, b, c, d, e, f, g, h, m, v1, v2;
	gsl_matrix *A = gsl_matrix_calloc(7, 7);
	gsl_vector *B = gsl_vector_calloc(7);
	gsl_vector *Y2 = gsl_vector_calloc(7);
	gsl_vector *F2 = gsl_vector_calloc(7);

	T1 = (Param->L1)/(Param->R1);
	T2 = (Param->L2)/(Param->R2);

	/* Escorregamento */
	w2=-abs((Param->P)*Y[6]);

	/* Conjugado eletromagnético */
	md= -((Param->P) * (Param->LH) /sqrt(3)) *(Y[3]*(Y[2]-Y[1]) + Y[4]*(Y[0]-Y[2]) + Y[5]*(Y[1]-Y[0])) - Param->MT; 

	/* Coeficientes da Matriz A */
	/* T1 = L1/R1
	 T2 = L2/R2 */
	a=-1/((Param->sig)*(T1));
	b=1/(sqrt(3)*(Param->sig))*w2*((Param->sig)-1);
	c=(Param->LH)/((T2)*(Param->sig)*(Param->L1));
	d=-(Param->LH)/((Param->sig)*sqrt(3)*(Param->L1))*w2;
	e=(Param->R1)*(1-(Param->sig))/((Param->sig)*(Param->LH));
	f=(1-(Param->sig))/((Param->sig)*sqrt(3))*(Param->L1)/(Param->LH)*w2;
	g=-1/((Param->sig)*(T2));
	h=1/(sqrt(3)*(Param->sig))*(w2);
	m=-(Param->KD)/(Param->JJ);
	v1=1/((Param->sig)*(Param->L1));
	v2=-(1-(Param->sig))/((Param->sig)*(Param->LH));
/*
	yp = [a  b -b  c  d -d 0 ;...
         -b  a  b -d  c  d 0 ;...
          b -b  a  d -d  c 0 ;...
          e  f -f  g  h -h 0 ;...
         -f  e  f -h  g  h 0 ;...
          f -f  e  h -h  g 0 ;...
          0  0  0  0  0  0 m]*y + ...
[v1*u1a; v1*u1b; v1*u1c; v2*u1a; v2*u1b; v2*u1c; md/JJ];
*/
	
	gsl_matrix_set (A, 0, 0, a);
	gsl_matrix_set (A, 0, 1, b);
	gsl_matrix_set (A, 0, 2, -b);
	gsl_matrix_set (A, 0, 3, c);
	gsl_matrix_set (A, 0, 4, d);
	gsl_matrix_set (A, 0, 5, -d);

	gsl_matrix_set (A, 1, 0, -b);
	gsl_matrix_set (A, 1, 1, a);
	gsl_matrix_set (A, 1, 2, b);
	gsl_matrix_set (A, 1, 3, -d);
	gsl_matrix_set (A, 1, 4, c);
	gsl_matrix_set (A, 1, 5, d);

	gsl_matrix_set (A, 2, 0, b);
	gsl_matrix_set (A, 2, 1, -b);
	gsl_matrix_set (A, 2, 2, a);
	gsl_matrix_set (A, 2, 3, d);
	gsl_matrix_set (A, 2, 4, -d);
	gsl_matrix_set (A, 2, 5, c);

	gsl_matrix_set (A, 3, 0, e);
	gsl_matrix_set (A, 3, 1, f);
	gsl_matrix_set (A, 3, 2, -f);
	gsl_matrix_set (A, 3, 3, g);
	gsl_matrix_set (A, 3, 4, h);
	gsl_matrix_set (A, 3, 5, -h);

	gsl_matrix_set (A, 4, 0, -f);
	gsl_matrix_set (A, 4, 1, e);
	gsl_matrix_set (A, 4, 2, f);
	gsl_matrix_set (A, 4, 3, -h);
	gsl_matrix_set (A, 4, 4, g);
	gsl_matrix_set (A, 4, 5, h);

	gsl_matrix_set (A, 5, 0, f);
	gsl_matrix_set (A, 5, 1, -f);
	gsl_matrix_set (A, 5, 2, e);
	gsl_matrix_set (A, 5, 3, h);
	gsl_matrix_set (A, 5, 4, -h);
	gsl_matrix_set (A, 5, 5, g);

	gsl_matrix_set (A, 6, 6, m);


	gsl_vector_set (B, 0, v1*Param->u1a);
	gsl_vector_set (B, 1, v1*Param->u1b);
	gsl_vector_set (B, 2, v1*Param->u1c);
	gsl_vector_set (B, 3, v2*Param->u1a);
	gsl_vector_set (B, 4, v2*Param->u1b);
	gsl_vector_set (B, 5, v2*Param->u1c);
	gsl_vector_set (B, 6, md/Param->JJ);

	for (z = 0; z < 7; z++) 
		gsl_vector_set(Y2, z, Y[z]);

	gsl_blas_dgemv(CblasNoTrans, 1.0, A, Y2, 0.0, F2);  /* F2[] = A[][]*Y[]  */
	gsl_vector_add (F2, B);  /* F2[] = F2[] + B */

	for (z = 0; z < 7; z++) 
		F[z] = gsl_vector_get(F2, z);

	gsl_vector_free (Y2);
	gsl_vector_free (F2);
	gsl_vector_free (B);
	gsl_matrix_free (A);
	
	return GSL_SUCCESS;
}

bool InductionMachine::Compute_Ih(bool e) {

	struct IM_ODE_Data Parameters = (struct IM_ODE_Data)data;
	Parameters.u1a= this->Get_V(0);
	Parameters.u1b= this->Get_V(1);
	Parameters.u1c= this->Get_V(2);

	double y2[7];

	y2[0] = this->y[0]; y2[1] = this->y[1]; y2[2] = this->y[2]; y2[3] = this->y[3];
	y2[4] = this->y[4]; y2[5] = this->y[5]; y2[6] = this->y[6];
	

	gsl_odeiv2_system sys = {func, NULL, 7, &Parameters };
	gsl_odeiv2_driver*d = gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rkf45, 1E-8, 1E-6, 0.0);
	gsl_odeiv2_driver_alloc_y_new (&sys, gsl_odeiv2_step_rk8pd,1e-6, 1e-6, 0.0);
	

	int i, status;

	status = gsl_odeiv2_driver_apply (d, &T, T + data.dt, y2);

	if (status != GSL_SUCCESS) {
		printf("error, return value = %d\n", status);
		return 1;
	}

	gsl_vector_set(&View_I_Hist_Pri.vector,0, y2[0]+Parameters.u1a/(2*data.LH/data.dt));
	gsl_vector_set(&View_I_Hist_Pri.vector,1, y2[1]+Parameters.u1b/(2*data.LH/data.dt));
	gsl_vector_set(&View_I_Hist_Pri.vector,2, y2[2]+Parameters.u1c/(2*data.LH/data.dt));

	this->y[0] = y2[0]; this->y[1] = y2[1]; this->y[2] = y2[2]; this->y[3] = y2[3];
	this->y[4] = y2[4]; this->y[5] = y2[5]; this->y[6] = y2[6];
		
	gsl_odeiv2_driver_free(d);
	return 0;
}

double InductionMachine::Get_Speed() {
	return this->y[6];
}
void InductionMachine::Set_Mec_Torque(double mt) {
	data.MT = mt;	
}
double InductionMachine::Get_Torque(){
	return -((data.P) * (data.LH) /sqrt(3)) *(this->y[3]*(this->y[2]-this->y[1]) + this->y[4]*(this->y[0]-this->y[2]) + this->y[5]*(this->y[1]-this->y[0])) ;
}







