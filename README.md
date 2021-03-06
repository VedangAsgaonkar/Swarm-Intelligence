# Swarm-Intelligence
This repository stores my implementation of ant-pheromone based swarm intelligence algorithms for finding optimal paths through obstacle ridden topologies. It
demonstrates local intelligence. The ants move throughout the space and as soon as a target is found, pheromone is deposited along the path. The quantity deposited 
depends inversely to the length of the path taken. Also, points closer to the target get more pheromone. Subsequent releases of the ants are biased in direction
of maximum pheromone. The model is observed to overcome local extrema quite successfully.
For more details of implementation, read logs in the code.
Stable models are v0, v1, v3, v5, v6, v8, v9, v10, v11

## Images
Some images of different version in action
### v11
| Source | Exploration | Target | Path |
| --- | --- | --- | --- |
| <img src="images/source_v11.png" width=200 height=200> | <img src="images/explore_v11.png" width=200 height=200> | <img src="images/target_v11.png" width=200 height=200> | <img src="images/path_v11.png" width=200 height=200> |

### v8
| Exploration | Path |
| --- | --- |
| <img src="images/explore_v8.png" width=200 height=200> | <img src="images/path_v8.png" width=200 height=200> |

### v5
| Exploration | Path |
| --- | --- |
| <img src="images/explore_v5.png" width=200 height=200> | <img src="images/path_v5.png" width=200 height=200> |

## Installation and Running
### For Linux 
Install g++ on your device and add its path to the environment variable. Install fltk:
```
sudo apt-get install libfltk1.3
```
Clone the v11.cpp and open terminal in the workind directory. Run the simulation using
```
fltk-config --compile v11.cpp
```
```
./v11
```
