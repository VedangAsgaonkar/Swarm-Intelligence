# Swarm-Intelligence
This repository stores my implementation of ant-pheromone based swarm intelligence algorithms for finding optimal paths through obstacle ridden topologies. It
demonstrates local intelligence. The ants move throughout the space and as soon as a target is found, pheromone is deposited along the path. The quantity deposited 
depends inversely to the length of the path taken. Also, points closer to the target get more pheromone. Subsequent releases of the ants are biased in direction
of maximum pheromone. The model is observed to overcome local extrema quite successfully.
For more ddetails of implementation, read logs in the code.
Stable models are v0, v1, v3, v5, v6, v8, v9, v10, v11
