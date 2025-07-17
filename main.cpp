#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/**
 * Win the water fight by controlling the most territory, or out-soak your opponent!
 **/
struct Position{
    Position(int x, int y): x{x}, y{y} {}
    int x;
    int y;
};

int manhattan(Position p1, Position p2)
{
    return abs(p2.y - p1.y) + abs(p2.x - p1.x);
}

class Agent{
    Agent(int id, int player, int optimal_range):
        id{id}, player{player}, pos{0,0},
        optimal_range{optimal_range}, wetness{0} 
        {}
    int id;
    int player;
    Position pos;
    int wetness;
    int optimal_range;

    // Methods
    bool is_in_range(const Agent& target){
        int dist = manhattan(this.pos, target.pos);
        if (dist > this.optimal_range)
            return false;
        else
            return true;
    }

    int closest_target(const Agent& target){

    }


};


Position move(int agent_id)
{
    if(agent_id == 0)
    {
        return Position(6,1);
    }
    else if(agent_id == 1)
    {
        return Position(6,3);
    }
    else
    {
        return Position(0,0);
    }
}

void printPosition(Position p)
{
    std::cout << "MOVE" << " " << p.x << " " << p.y << "\n";
}

int main()
{
    int my_id; // Your player id (0 or 1)
    cin >> my_id; cin.ignore();
    int agent_data_count; // Total number of agents in the game
    cin >> agent_data_count; cin.ignore();

    // Adding enemies agents id
    std::map<int,Agent> enemies;
    std::map<int,Agent> friends;

    for (int i = 0; i < agent_data_count; i++) {
        int agent_id; // Unique identifier for this agent
        int player; // Player id of this agent
        int shoot_cooldown; // Number of turns between each of this agent's shots
        int optimal_range; // Maximum manhattan distance for greatest damage output
        int soaking_power; // Damage output within optimal conditions
        int splash_bombs; // Number of splash bombs this can throw this game
        cin >> agent_id >> player >> shoot_cooldown >> optimal_range >> soaking_power >> splash_bombs; cin.ignore();

        if(player != my_id)
        {
            enemies[agent_id] = Agent(agent_id,player,optimal_range);
        } else {
            friends[agent_id] = Agent(agent_id,player,optimal_range);
        }
    }
    int width; // Width of the game map
    int height; // Height of the game map
    cin >> width >> height; cin.ignore();
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int x; // X coordinate, 0 is left edge
            int y; // Y coordinate, 0 is top edge
            int tile_type;
            cin >> x >> y >> tile_type; cin.ignore();
        }
    }

    // game loop
    while (1) {
        int agent_count; // Total number of agents still in the game
        cin >> agent_count; cin.ignore();
        for (int i = 0; i < agent_count; i++) {
            int agent_id;
            int x;
            int y;
            int cooldown; // Number of turns before this agent can shoot
            int splash_bombs;
            int wetness; // Damage (0-100) this agent has taken
            cin >> agent_id >> x >> y >> cooldown >> splash_bombs >> wetness; cin.ignore();

            if (enemies.find(agent_id) != enemies.end())
            {
                enemies[agent_id].pos = Position(x,y);
                enemies[agent_id].wetness = wetness;
            } else {
                friends[agent_id].pos = Position(x,y);
            }
        }
        int my_agent_count; // Number of alive agents controlled by you
        cin >> my_agent_count; cin.ignore();
        for (int i = 0; i < my_agent_count; i++) {

            // Write an action using cout. DON'T FORGET THE "<< endl"
            // To debug: cerr << "Debug messages..." << endl;

            // First policy: if agent is outside of optimal range, move, else attack
            Agent& ag = friends[i];
            Agent& target = ag.closest_enemy(enemies);

            if(ag.is_in_range(target))
            {
                ag.shoot(target); //TODO: position?
            } else {
                ag.move(target);
            }

            // One line per agent: <agentId>;<action1;action2;...> actions are "MOVE x y | SHOOT id | THROW x y | HUNKER_DOWN | MESSAGE text"
        }
    }
}
