# ms-van3t

![](img/MS-VAN3T_logo-V2_small.png)

ns-3 modules to build and simulate ETSI-compliant VANET (V2X) applications using SUMO (v-1.6.0+) and ns-3 (ns-3-dev, version supporting the NR-V2X module by CTTC), with the possibility of easily switching stack and communication technology.

It has been tested with SUMO v1.6.0, v1.7.0, v1.8.0, v1.12.0, v1.18.0 on Ubuntu 20.04 and 22.04.
Back compatibility **is not** ensured with new versions of TraCI.

To build the project:
* Install SUMO following the guide at [https://sumo.dlr.de/wiki/Downloads](https://sumo.dlr.de/wiki/Downloads)
    * You can use 
    
    	`sudo add-apt-repository ppa:sumo/stable`  
    	`sudo apt update`  
    	`sudo apt install sumo sumo-tools sumo-doc`  
    * Be careful: in the future the previous commands will install updated version of SUMO which are not ensured to work with this scripts (that are tested with any version from **v-1.6.0** to **v-1.18.0** )
    * Test sumo by opening a terminal and running "sumo-gui".
	
    * **Possible problems**:
			
			You may get the following error when running SUMO:
			
        	"sumo-gui: symbol lookup error: /usr/lib/libgdal.so.26: undefined symbol: GEOSMakeValid_r"
    
        	To solve it, remove all the reference to GEOS inside /usr/local/lib/ (do NOT do it if you need the GEOS library):
    
        	"sudo rm /usr/local/lib/libgeos*"

* Clone this repository in your pc:

`git clone https://github.com/ms-van3t-devs/ms-van3t.git`

* Run, from this repository either:

`./sandbox_builder.sh install-dependencies` -> if this is the first time you install ns-3 or ms-van3t on your system

or

`./sandbox_builder.sh` -> if this is **not** the first time you install ns-3 

This script will download the proper version of ns-3-dev and install this framework. The folder `ns-3-dev` will remain linked to this GitHub repository (not to the vanilla ns-3-dev one), allowing you to more easily develop updates and possibile contributions to *ms-van3t*.
    
* Configure `ns3` to build the framework with `<ns3-folder>./ns3 configure --build-profile=optimized --enable-examples --enable-tests --disable-python --disable-werror (add here what you want to enable)"` - The usage of the optimized profile allows to speed up the simulation time. This command should be launched from inside the `ns-3-dev` folder.


* Build ns3:
`./ns3 build`
## Acknowledgements

To acknowledge us in your publication(s) please refer to the following publication:

```tex
@inproceedings{10.1145/3416014.3424603,
	author = {Malinverno, Marco and Raviglione, Francesco and Casetti, Claudio and Chiasserini, Carla-Fabiana and Mangues-Bafalluy, Josep and Requena-Esteso, Manuel},
	title = {A Multi-Stack Simulation Framework for Vehicular Applications Testing},
	year = {2020},
	isbn = {9781450381215},
	publisher = {Association for Computing Machinery},
	address = {New York, NY, USA},
	url = {https://doi.org/10.1145/3416014.3424603},
	doi = {10.1145/3416014.3424603},
	booktitle = {Proceedings of the 10th ACM Symposium on Design and Analysis of Intelligent Vehicular Networks and Applications},
	pages = {17–24},
	numpages = {8},
	keywords = {vehicular networks, sumo, V2X, NS-3, 802.11p, ETSI facilities layer, C-V2X},
	location = {Alicante, Spain},
	series = {DIVANet '20}
}
```
