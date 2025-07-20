#include <algorithm>
#include <iostream>
#include <map>
#include <limits>
#include <optional>
#include <stdexcept>
#include <optional>
#include <sstream>

using namespace std;

/**
 * Win the water fight by controlling the most territory, or out-soak your opponent!
 **/

struct Position{
    Position() = default;
    Position(int x, int y): x{x}, y{y} {}
    int x;
    int y;

    friend bool operator==(const Position& lhs, const Position& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    friend Position operator-(const Position& lhs, const Position& rhs){
        return Position(lhs.x - rhs.x, lhs.y-rhs.y);
    }

};

int manhattan(Position p1, Position p2){
    return abs(p2.y - p1.y) + abs(p2.x - p1.x);
}

class Grid{
    public:
        std::map<Position,int> grid;
        int height;
        int width;

        bool is_inside(Position& pos){
            return pos.x > 0 && pos.x < width && pos.y > 0 && pos.y < height;
        }

        std::optional<int> at(const Position& pos){
            return at(pos.x,pos.y);
        }

        std::optional<int> at(int x, int y) { 
            auto pos = Position(x,y);
            
            if(is_inside(pos))
            {
                return grid[Position(x,y)];
            }

            return std::nullopt;
        }

        bool isCover(const Position& pos)
        {
            const auto cover = this->at(pos);
            if(cover)
            {
                return cover == 1 || cover == 2;
            } else {
                return false;
            }
        }
};

class Agent{

    public:
        Agent() = default;
        Agent(int id, int player, int optimal_range, Grid& grid):
            id{id}, player{player}, pos{0,0},
            optimal_range{optimal_range}, wetness{0}, is_alive{true},
            grid{grid}
            {}
        Agent(const Agent& ag):
            id{ag.id}, player(ag.player), pos(ag.pos.x,ag.pos.y),
            optimal_range(ag.optimal_range), wetness{ag.wetness}, is_alive{true}
            {}

        int id;
        int player;
        Position pos;
        int wetness;
        int optimal_range;
        bool is_alive;
        Grid grid;
        std::stringstream action;

        // Methods
        bool is_in_range(const Agent& target){
            int dist = manhattan(this->pos, target.pos);
            if (dist > this->optimal_range)
                return false;
            else
                return true;
        }

        const Agent& wettest_target(const std::map<int,Agent>& targets) const {

            if(targets.empty())
            {
                throw runtime_error("targets should never be empty");
            }

            int max_wet = std::numeric_limits<int>::min();
            int wet_idx = -1;
            for(const auto& [target_idx,target]: targets)
            {   
                if(!target.is_alive)
                    continue;

                if(target.wetness > max_wet)
                {
                    max_wet = target.wetness ;
                    wet_idx = target_idx;
                }
            }

            return targets.at(wet_idx);
        }

        const Agent& closest_target(const std::map<int,Agent>& targets) const {

            if(targets.empty())
            {
                throw runtime_error("targets should never be empty");
            }

            int min_dist = std::numeric_limits<int>::max();
            int min_idx = -1;
            for(const auto& [target_idx,target]: targets)
            {   
                std::cerr << "dist:" << manhattan(this->pos,target.pos) << "\n";
                std::cerr << "target_idx: " << target_idx << "\n";
                if(int dist = manhattan(this->pos,target.pos) < min_dist)
                {
                    min_dist = dist;
                    min_idx = target_idx;
                }
            }

            std::cerr << "min_idx: " << min_idx;
            return targets.at(min_idx);
        }

        const Agent& closest_exposed_target(const std::map<int,Agent>& targets) const 
        {
            int min_dist = std::numeric_limits<int>::max();
            int cover_closest_enemy = 2;
            int enemy_idx;

            for(const auto& [id,target]: targets)
            {
                auto cover = target.highest_target_cover(this->pos);
                if( cover.second < cover_closest_enemy) {
                    min_dist = manhattan(target.pos, this->pos);
                    enemy_idx = id;
                } else if(cover.second == cover_closest_enemy) {
                    if(int dist = manhattan(target.pos, this->pos) < min_dist){
                        min_dist = dist;
                        enemy_idx = id;
                    }
                }
            }

            return targets.at(enemy_idx);
        }

        const Position highest_closest_cover() const {
            int min_dist = std::numeric_limits<int>::max();
            Position closest_highest_cover;
            int min_tile_type = 0;
            for(const auto& [pos,type]: this->grid){
                
                // If we get a better cover we switch
                if(type > min_tile_type){
                    min_dist = manhattan(this->pos, pos);
                    min_tile_type = type;
                    closest_highest_cover = pos;
                } else if (type == min_tile_type) {
                    if(int dist = manhattan(this->pos, pos) < min_dist)
                    {   
                        min_dist = dist;
                        closest_highest_cover = pos;
                    }
                }
            }
            
            return closest_highest_cover;
        }

        std::map<Position,int> covers(const Position& pos) const {
            // check for covers to which is orthogonal currently

            std::map<Position,int> covers;

            for(const auto x_dir: {-1,0,1}){
                for(const auto y_dir: {-1,0,1}){
                    
                    auto check_pos = Position(pos.x + x_dir, pos.y + y_dir);

                    if(check_pos == pos)
                        continue;
                    
                    if(grid.isCover(check_pos)){
                        //check that this is on the other side
                        if(this->pos.x < check_pos.x && check_pos.x < pos.x)
                            covers[check_pos] = this->grid[check_pos];
                        else if(this->pos.y < check_pos.y && check_pos.y < pos.y)
                            covers[check_pos] = this->grid[check_pos];
                        else if(this->pos.x > check_pos.x && check_pos.x > pos.x)
                            covers[check_pos] = this->grid[check_pos];
                        else if(this->pos.y < check_pos.y && check_pos.y < pos.y)
                            covers[check_pos] = this->grid[check_pos];
                    }
                }
            }

            return covers;
        }

        std::pair<Position,int> highest_target_cover(const Position& pos) const {
        
            auto covers = this->covers(pos);
            auto best = std::max_element(covers.begin(),covers.end(),
        [](const std::pair<Position,int> p1, const std::pair<Position,int> p2){ return p1.second < p2.second;});

            return std::make_pair(best->first,best->second);
        }

        void act() const {
            std::cout << action.str();
        }

        void shoot(const Agent& target) {
            action << " SHOOT" << " " << target.id << "\n";
        }

        void move(const Position& pos) {
            action << "MOVE " << pos.x << " " << pos.y ;
        }
};


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

    std::map<Position,int> grid;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int x; // X coordinate, 0 is left edge
            int y; // Y coordinate, 0 is top edge
            int tile_type;
            cin >> x >> y >> tile_type; cin.ignore();
            std::cerr << "x:" << x << "y:" << y << "\n";
            grid[Position(x,y)] = tile_type;
        }
    }




    // game loop
    while (1) {
        int agent_count; // Total number of agents still in the game
        cin >> agent_count; cin.ignore();

        // Set agents to dead before we get any info regarding them
        for (auto& [_,agent]: friends)
            agent.is_alive = false;

        for (auto& [_,agent]: enemies)
            agent.is_alive = false;


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
                enemies[agent_id].is_alive = true;
                // std::cerr << "wetness of enemy: " << agent_id << ": " << wetness << "\n";
            } else {
                friends[agent_id].pos = Position(x,y);
                friends[agent_id].is_alive = true;
            }
        }
        int my_agent_count; // Number of alive agents controlled by you
        cin >> my_agent_count; cin.ignore();

        for (auto& [id,ag]: friends )
        {
            if(ag.is_alive){
                // const Agent& target = ag.closest_target(enemies);
                // const Agent& target = ag.wettest_target(enemies);
                const Position cover = ag.highest_closest_cover();
                
                // To understand where to position we need to be orthogonal with respect to the cover
                // and the enemy agent we intend to shoo
                const Agent& target = ag.closest_exposed_target(enemies);

                
                // Move to other side of cover w.r.t target
                Position v = target.pos - cover;

                if(v.x > 0){
                    ag.move(Position(cover.x-1,cover.y));
                } else if (v.x < 0) {
                    ag.move(Position(cover.x + 1, cover.y));
                } else if (v.y > 0) {
                    ag.move(Position(cover.x, cover.y - 1));
                } else if (v.y < 0) {
                    ag.move(Position(cover.x, cover.y + 1))
                }
                
                ag.shoot(target);

                ag.act();
            }

        }
    }
}
