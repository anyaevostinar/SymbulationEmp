/* Made: June 25, 2021
   Author: Tiffany-Ellen Vo
   Purpose: Have host and symbiont to interact with each other using one unique instruction for each (host_donate, sym_donate)

*/

//header files----------------------------------------------------------------------------------
#include "AvidaGPHost.hpp"
#include "AvidaGPSymbio.hpp"

//create new instance variable in host and symbionts called points
    //should this be done in: AvidaGPHost file and AvidaSymGP file?

//hosts can be born with 50 points for now just to test 
    //just set points = 50

//the fitness function is the result of the squares + the points of the host

//two instruction sets, one for host and one for symbionts
// should be done in their own functions
//host_donate: donates a point to the symbiont
void host_donate(host, symbiont){
    // add a point to the symbiont
}
//sym_donate: donates a point to the host that is then multiplied by 3
void sym_donate(host, symbiont){
    //host point += (1*3)??
}

