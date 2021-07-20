#ifndef ORGANISM_H
#define ORGANISM_H

#include <set>
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include "ConfigSetup.h"

class Organism {

  public:

  Organism() = default;
  Organism(const Organism &) = default;
  Organism(Organism &&) = default;
  virtual ~Organism() {}
  Organism & operator=(const Organism &) = default;
  Organism & operator=(Organism &&) = default;
  bool operator==(const Organism &other) const {return (this == &other);}
  bool operator!=(const Organism &other) const {return !(*this == other);}

  virtual double GetIntVal() const {
    std::cout << "GetIntVal called from Organsim" << std::endl;
    throw "Organism method called!";}
  virtual double GetPoints() {
    std::cout << "GetPoints called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void SetIntVal(double _in) {
    std::cout << "SetIntVal called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void SetPoints(double _in) {
    std::cout << "SetPoints called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void AddPoints(double _in) {
    std::cout << "AddPoints called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void SetHost(emp::Ptr<Organism> _in) {
    std::cout << "SetHost called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void SetDead() {
    std::cout << "SetDead called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual bool GetDead() {
    std::cout << "GetDead called from Organism" << std::endl;
    throw "Organism method called!";
  }
  virtual void Process(size_t location)
   {std::cout << "Process called from Organism" << std::endl;
     throw "Organism method called!";}

  //Symbiont functions

  virtual void HorizMutate() {
    std::cout << "HorizMutate called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void mutate() {
    std::cout << "mutate called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual emp::Ptr<Organism> reproduce() {
    std::cout << "reproduce called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void VerticalTransmission(emp::Ptr<Organism> host_baby) {
    std::cout << "reproduce called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual double ProcessResources(double sym_piece){
    std::cout << "ProcessResources called from an Organism" << std::endl;
    throw "Organism method called!";}
  virtual bool IsPhage(){
    std::cout << "IsPhage called from an Organism" << std::endl;
    throw "Organism method called!";}   
  virtual emp::Ptr<Organism> GetHost() {
    std::cout << "GetHost called from an Organism" << std::endl;
    throw "Organism method called!";}

  //EfficientSymbiont functions
  virtual double GetEfficiency() {
    std::cout << "GetEfficiency called from Organism" << std::endl;
    throw "Organism method called!";
  }

  //Host functions

  virtual emp::vector<emp::Ptr<Organism>>& GetSymbionts() {
    std::cout << "GetSymbionts called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual emp::vector<emp::Ptr<Organism>>& GetReproSymbionts() {
    std::cout << "GetReproSymbionts called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual std::set<int> GetResTypes() const {
    std::cout << "GetResTypes called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void SetSymbionts(emp::vector<emp::Ptr<Organism>> _in) {
    std::cout << "SetSymbionts called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void SetResTypes(std::set<int> _in) {
    std::cout << "SetResTypes called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void AddSymbiont(emp::Ptr<Organism> _in)
   {std::cout << "AddSymbiont called from Organism" << std::endl;
     throw "Organism method called!";}
  virtual void AddReproSym(emp::Ptr<Organism> _in) {
    std::cout << "AddReproSym called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual bool HasSym() {
    std::cout << "HasSym called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual bool IsHost() {
    std::cout << "IsHost called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void DistribResources(double resources) {
    std::cout << "DistribResources called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void ClearSyms() {
    std::cout << "ClearSyms called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void ClearReproSyms() {
    std::cout << "ClearReproSyms called from Organism" << std::endl;
    throw "Organism method called!";}

  //Phage functions
  virtual double GetBurstTimer() {
    std::cout << "GetBurstTimer called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void IncBurstTimer() {
    std::cout << "IncBurstTimer called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void SetBurstTimer(int _in) {
    std::cout << "SetBurstTimer called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual double GetLysisChance() {
    std::cout << "GetLysisChance called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void SetLysisChance(double _in){
    std::cout << "SetLysisChance called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual void uponInjection() {
    std::cout << "chooseLysisOrLysogeny called from Organism" << std::endl;
    throw "Organism method called!";}
  virtual bool GetLysogeny() {
    std::cout << "GetLysogeny called from Organism" << std::endl;
    throw "Organism method called!";}

  //Social goods game host functions 
  virtual void SetPool() {
    std::cout << "Set rescources pool from Organism" << std::endl;
    throw "Organism method called!";}  
  virtual void AddPool() {
    std::cout << "Add rescources pool from Organism" << std::endl;
    throw "Organism method called!";} 
  virtual void DistribPool() {
    std::cout << "dis rescources pool from Organism" << std::endl;
    throw "Organism method called!";} 
  //Social goods game sym functions
  virtual double Getdonation() {
    std::cout << "Get rescources donation from Organism" << std::endl;
    throw "Organism method called!";}  
  virtual void Setdonation() {
    std::cout << "Set rescources pool from Organism" << std::endl;
    throw "Organism method called!";}  
  virtual double ProcessPool() {
    std::cout << "Dis called from Organism" << std::endl;
    throw "Organism method called!";}

};

#endif
