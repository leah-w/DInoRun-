#define _CRT_SECURE_NO_WARNINGS
#include "snail.cpp"
#include "cow.cpp"
#include "_cow_supplement.cpp"
#include "jim.cpp"
// #define EIGEN_LINEAR_SOLVER // will discuss Thursday
#include "_cow_optimization.cpp"

double t = 0; 
//have camera "track character", only x position 
//write shader to animate sprites , just loop throught array of sprites
//gui ?? health, coins , eh 
//collosion and physics , ahhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
struct {
    bool facing_right = true;
    bool idle = true; 
    bool jump = false; 
    bool win = false; 
    bool lose = false; 
} tweaks;

struct gameObj {
    vec3 position; //top left 
    vec3 size; 
    vec3 color; 
};

//the blocks that make up the scene
int num_blocks = 8; 
gameObj blocks[8] = {{V3(-15,-7,0.5), V3(25, -3, 0) ,V3(1./255, 69./255, 19./255)}, //ground
                    { V3(3,2,0.5), V3(1, -1, 0) , monokai.purple} , //floating purple block
                    { V3(10,-4,0.5), V3(6, -1, 0) , monokai.purple}, 
                    { V3(19,-6,0.5), V3(4, -2, 0) , monokai.purple},
                    { V3(25,-4,0.5), V3(1, -1, 0) , monokai.purple},
                    { V3(28,-2,0.5), V3(1, -1, 0) , monokai.purple}, 
                    { V3(36,-7,0.5), V3(8, -2, 0) , monokai.purple},
                    { V3(48,-5,0.5), V3(15, -2, 0) , monokai.purple} };

//https://arks.itch.io/dino-characters
int num_sprite = 7; 
int sprite_frame = 0; 
int frame = 0;
char *sprite_move[] = {"DinoSprite1.png", "DinoSprite4.png", "DinoSprite5.png", 
                    "DinoSprite6.png", "DinoSprite7.png", "DinoSprite8.png", "DinoSprite9.png"};
char *sprite_idle[] = {"DinoSprite1.png", "DinoSprite2.png", "DinoSprite3.png" };  
int spike_frame = 0; 
char *spike_anime[] = {"spike1.png", "spike2.png", "spike3.png", "spike4.png", "spike5.png"};
                       //"spike4.png", "spike3.png" "spike2.png"};                    

//camera follow character 
struct ScrollingCamera {
    vec3 origin;
    double angle_of_view;    
    vec3 *char_pos;
    mat4 R; 
};

mat4 scrolling_camera_get_C(ScrollingCamera *scroll) { 
    mat4 scrolling_c = Translation(scroll->origin) * scroll->R;  
    return scrolling_c; 
}

//axis-aligned bounding box colliosion check for rectangles
bool rect_colllsion(gameObj *character, gameObj *wall){  
    //if next move of charater would put in inside of another oject dont move
    //check for x-axis collision
    bool x_axis = wall->position.x + wall->size.x >= character->position.x &&
        character->position.x + character->size.x >= wall->position.x;    
    //check for y-axis collision 
    bool y_axis = wall->position.y  >= character->position.y + character->size.y &&
        character->position.y >= wall->position.y + wall->size.y ; 
    // collision only if on both axes
    return x_axis && y_axis; 
} 

//check is closes point on square to circle's distance to circle is less than radius
bool cir_colllsion(gameObj *character, gameObj *coin){ 
    //center point on circle 
    vec2 center = {coin->position.x + coin->size.x , coin->position.y + coin->size.y };
    //axis-aligned bounding box info , center and half-extents
    vec2 half_extents = {character->size.x / 2.0f, abs(character->size.y) / 2.0f};
    vec2 rect_center = {
        character->position.x + half_extents.x, 
        character->position.y - half_extents.y
        };
    //difference between two centers 
    vec2 difference = center - rect_center;
    vec2 clamped = { CLAMP(difference.x, -half_extents.x, half_extents.x ),
                    CLAMP(difference.y, -half_extents.y, half_extents.y)};
    //the value of box closest to circle
    vec2 closest = rect_center + clamped;
    //vector between circle center and closest rect point
    difference = closest - center;
    //check if length is less than radius
    return norm(difference) < coin->size.x; 
} 
/*
vec3* get_Position(vec3 pos, vec3 size){
    vec3 position[] = {
        { pos.x, pos.y, 0 },
        { pos.x + size.x, pos.y, 0 },
        { pos.x + size.x, pos.y + size.y, 0},
        { pos.x , pos.y + size.y, 0 }
    };
    return position; 
}
*/

//character movement 
void character_move(gameObj *character) {       
    //imgui_readout("y", &character->position.y); 
    //double dy = 0; double yVel = 0; 
    //imgui_readout("spriframe", &sprite_frame); 
    
    //animation idle and moving, jumping ??
    //frame++;
    if(input.key_held['A'] || input.key_held['D'] ){
        tweaks.idle = false; 
        sprite_frame = ((frame/5) % num_sprite); 
    }else{
        tweaks.idle = true; 
        sprite_frame = ((frame/9) % 3); 
    }

    if(input.key_held['A']){//left        
            //for(int i = 0; i < num_sprite; i ++){
        for(int i = 0; i < num_blocks; i ++){
            if(!rect_colllsion(character, &blocks[i])){
                tweaks.facing_right = false; 
                character->position.x -= 0.05; //= origin->x - 0.1*t;
            }else{
                character->position.x += TINY;
            }
        }       
    }

    if(input.key_held['D']){//right 
        for(int i = 0; i < num_blocks; i ++){
            if(!rect_colllsion(character, &blocks[i])){
                tweaks.facing_right = true;
                character->position.x += 0.05; //= origin->x - 0.1*t;
            }else{
                character->position.x -= TINY;
            }
        }
    }
    //double tempCrouch = character->size.y;
    if(input.key_held['S']){
        //character->position.y -= 0.2;
    }//else if(input.key_released){
        //character->size.y = tempCrouch*1;
    //}
    //floor is at -1
    if(input.key_pressed['W']){//jump
        tweaks.jump = true; 
    }
        
}

void final_proj() {
    init();
    gameObj character = { V3(-10,2,0.5), V3(1.5,-2, 0), monokai.orange};
    vec2 char_bounding_box[4] = { {2,2}, {2,1}, {1,1}, {1,2}}; 
    //vec2 char_pos[1] = {{1,1}}; 
    //vec3 color = monokai.orange; 
    //double time = 0;  
    //Camera3D camera = { 5, RAD(90) };    
    ScrollingCamera scroll = { V3( 0, 0, 10), RAD(90), &character.position , Identity4x4};
    int num_coins = 6; 
    gameObj coins[6]  = { {V3(6,1,0.5), V3(1, -1, 0), monokai.orange}, 
                        {V3(-5,-3,0.5), V3(1, -1, 0), monokai.orange}, 
                        {V3(17, 0,0.5), V3(1, -1, 0), monokai.orange}, 
                        {V3(30, 0.5,0.5), V3(1, -1, 0), monokai.orange} ,
                        {V3(39, -3.5,0.5), V3(1, -1, 0), monokai.orange} , 
                        {V3(48.5, -2.5,0.5), V3(1, -1, 0), monokai.orange} };
    int num_spikes = 8; 
    gameObj spikes[8] = { {V3(-3,-5.5,0.5), V3(1.5, -1.5, 0), monokai.orange}, 
                        { V3(4,-5.5,0.5), V3(1.5, -1.5, 0), monokai.orange} , 
                        { V3(13,-2.5,0.5), V3(1.5, -1.5, 0), monokai.orange} ,
                        { V3(21.5,-4.5,0.5), V3(1.5, -1.5, 0), monokai.orange} ,
                        { V3(36,-5.5,0.5), V3(1.5, -1.5, 0), monokai.orange} ,
                        { V3(42.5,-5.5,0.5), V3(1.5, -1.5, 0), monokai.orange} , 
                        { V3(52.5,-3.5,0.5), V3(1.5, -1.5, 0), monokai.orange} , 
                        { V3(54,-3.5,0.5), V3(1.5, -1.5, 0), monokai.orange} };
    //flag image: https://www.vecteezy.com/vector-art/14446095-pixel-art-flagpole-with-red-flag-vector-icon-for-8bit-game-on-white-background
    gameObj flag = { V3(61.5,-0.5,0.5), V3(2, -4.5, 0), monokai.orange};                       
    //for gui
    vec3 box_pos[] = { {-1, 1 , 0}, {-.65 , 1, 0}, {-.65, 0.6, 0}, {-1, 0.6, 0}};

    int health = 5; 
    int points = 0; 

    double jumpHeight = 0; 
    double maxHeight = 5;
    
    //int num_vertices = 4;
    //int num_triangles = 2;
    int3 triangle_indices[] = {
        { 0, 1, 2 },
        { 0, 2, 3 }
    };
    vec3 background_positions[] = {
        { -20,  10, 0 },
        {  20,  10, 0 },
        {  20, -10, 0 },
        { -20, -10, 0 }
    };

    vec3 NDC_coords[] = {
        {  -1,  1, 1 },
        {   1,  1, 1 },
        {   1, -1, 1 },
        {  -1, -1, 1 }
    };

    vec2 vertex_texCoords[] = {
        { 0, 1 },
        { 1, 1 },
        { 1, 0 },
        { 0, 0 }
    };

    vec2 char_vertex_left[] = {
        { 1, 1 },
        { 0, 1 },
        { 0, 0 },
        { 1, 0 }
    };

    vec2 char_vertex_right[] = {
        { 0, 1 },
        { 1, 1 },
        { 1, 0 },
        { 0, 0 }
    }; 
 
    
    while (begin_frame()) {
        //static Camera3D camera = { 5, RAD(45) };
        //static TrackingCamera track = { V3( 0, 0, 0), RAD(90), &character.position };
        //camera_move(&camera);
        //gl_PV(camera_get_PV(&track));
        //mat4 PV = camera_get_PV(&track);
        //vec3 back_color = plasma(t); 
        //clear_draw_buffer(back_color.x, back_color.y, back_color.z, 1.0); 
        clear_draw_buffer(0.0, 0.0, 0.0, 0.0);
        mat4 C = scrolling_camera_get_C(&scroll); 
        mat4 P = tform_get_P_perspective(scroll.angle_of_view); 
        mat4 V = inverse(C);

        frame++;

        //sudo gui 
        textColor = V3(1, 0, 0); 
        imgui_readout("HEALTH:", &health);
        textColor = V3(0, 1, 0); 
        imgui_readout("POINTS:", &points);
        //basic_text(*(P*V), "hello", 0, 0, 0, 0, 0, 0, 1); 
        //tile background ? 
        
        fancy_draw(Identity4x4, Identity4x4, Identity4x4,
                2, triangle_indices,
                4, NDC_coords,
                NULL, NULL, {},
                vertex_texCoords, "Landscape.png");
        //gui cover 
        basic_draw(QUADS, Identity4x4, 4, box_pos, V3(0,0,0)); 

        for(int i = 0; i < num_blocks; i ++){// write scence
            gameObj floor = blocks[i]; 
            vec3 vertex_positions[] = {
                { floor.position.x, floor.position.y, 0 },
                { floor.position.x + floor.size.x, floor.position.y, 0 },
                { floor.position.x + floor.size.x, floor.position.y + floor.size.y, 0},
                { floor.position.x , floor.position.y + floor.size.y, 0 }
            };
            fancy_draw(P, V, Identity4x4,
                2, triangle_indices,
                4, vertex_positions,
                NULL, NULL, {},
                vertex_texCoords, "ground.jpg");
        }   
        
        vec3 char_vertex[4] = {V3(character.position.x, character.position.y, 0),  
                                V3(character.position.x + character.size.x, character.position.y, 0), 
                                V3(character.position.x + character.size.x, character.position.y + character.size.y, 0), 
                                V3(character.position.x , character.position.y + character.size.y, 0)}; 

        if(!tweaks.lose && !tweaks.win){
            character_move(&character);
            scroll.origin.x = character.position.x; 
        }       

        //make vector of differetn spirites and loop between them 
        //imgui_readout("idle", &tweaks.idle); 
        if(tweaks.idle){
            if(tweaks.facing_right){
                fancy_draw(P, V, Identity4x4,
                    2, triangle_indices,
                    4, char_vertex,
                    NULL, NULL, {},
                    char_vertex_right, sprite_idle[sprite_frame]);  
            }else{
                fancy_draw(P, V, Identity4x4,
                    2, triangle_indices,
                    4, char_vertex,
                    NULL, NULL, {},
                    char_vertex_left, sprite_idle[sprite_frame]); 
            }   
        }else{
            if(tweaks.facing_right){
                fancy_draw(P, V, Identity4x4,
                    2, triangle_indices,
                    4, char_vertex,
                    NULL, NULL, {},
                    char_vertex_right, sprite_move[sprite_frame]);  
            }else{
                fancy_draw(P, V, Identity4x4,
                    2, triangle_indices,
                    4, char_vertex,
                    NULL, NULL, {},
                    char_vertex_left, sprite_move[sprite_frame]); 
            }   
        }

        //jump    
        if(tweaks.jump){
            //character->position.y += 0.2;
            if(jumpHeight < maxHeight){
                jumpHeight += 0.2; 
                character.position.y += 0.4;
            }else if( jumpHeight > maxHeight){
                //jumpHeight += 0.2; 
            }   
        }        
        character.position.y -= 0.2;
        bool hitBlock = false; 
        for(int i = 0; i < num_blocks; i ++){
            if(rect_colllsion(&character, &blocks[i])){
                hitBlock = true; 
                jumpHeight = 0; 
                tweaks.jump = false; 
            }
        }
        if(hitBlock){
            character.position.y += 0.2;
        }
        
        //coin draw 
        for(int i = 0; i < num_coins; i ++){
            gameObj coin = coins[i];
            vec3 coin_positions[] = {
                { coin.position.x, coin.position.y, 0 },
                { coin.position.x + coin.size.x, coin.position.y, 0 },
                { coin.position.x + coin.size.x, coin.position.y + coin.size.y, 0},
                { coin.position.x , coin.position.y + coin.size.y, 0 }
            };
            //coin.position.y += (sin(t))/60.0; 
            fancy_draw(P, V, Identity4x4,
                    2, triangle_indices,
                    4, coin_positions,
                    NULL, NULL, {},
                    vertex_texCoords, "coin.png");
            
            if(cir_colllsion(&character, &coin)){
                //remove coin from array 
                points ++;
                num_coins --; 
                for(int j = i; j < num_coins; j ++){
                    coins[j] = coins[j+1]; 
                }                    
            } 
        }
        for(int i = 0; i < num_coins; i ++){
            coins[i].position.y += (sin(t))/60.0; 
        }
       
        spike_frame = ((frame/6) % 5); 
        //spikes draw 
        for(int i = 0; i < num_spikes; i ++){
            gameObj spike = spikes[i]; 
            vec3 spike_positions[] = {              
                { spike.position.x, spike.position.y, 0 },
                { spike.position.x + spike.size.x, spike.position.y, 0 },
                { spike.position.x + spike.size.x, spike.position.y + spike.size.y, 0},
                { spike.position.x , spike.position.y + spike.size.y, 0 }
            };    
            fancy_draw(P, V, Identity4x4,
                2, triangle_indices,
                4, spike_positions,
                NULL, NULL, {},
                vertex_texCoords, spike_anime[spike_frame]);

            if(rect_colllsion(&spike, &character)){ 
                health --; 
                if(tweaks.facing_right){
                    character.position.x -= 0.6;
                    character.position.y += 0.3;
                }else{
                    character.position.x += 0.6;
                    character.position.y += 0.3;
                }
            }
        }

        //draw flag 
        vec3 flag_positions[] = {              
                { flag.position.x, flag.position.y, 0 },
                { flag.position.x + flag.size.x, flag.position.y, 0 },
                { flag.position.x + flag.size.x, flag.position.y + flag.size.y, 0},
                { flag.position.x , flag.position.y + flag.size.y, 0 }
            };    
         fancy_draw(P, V, Identity4x4,
                2, triangle_indices,
                4, flag_positions,
                NULL, NULL, {},
                vertex_texCoords,  "flag.png");   
        if(rect_colllsion(&flag, &character)){ 
                tweaks.win = true; 
        } 

        if(character.position.y <= -17 && !tweaks.lose ){
            character.position = V3(-8,2,0.5); 
            health --; 
        }
        //end game 
        if(health <= 0){
            tweaks.lose = true; 
        }
        //lose screen 
        if(tweaks.lose){
            textColor = V3(0,0,0); 
            basic_text(NULL, "you lose", 350, 200, 0, textColor.r, textColor.g, textColor.b, 1, 90);
            basic_text(NULL, "press [q] to quit", 50, 400, 0, textColor.r, textColor.g, textColor.b, 1, 50);
            basic_text(NULL, "press [r] to retry", 470, 400, 0, textColor.r, textColor.g, textColor.b, 1, 50);
            if(input.key_pressed['R']){
                character.position = V3(-10,2,0.5);
                scroll.origin.x = character.position.x; 
                health = 5; points = 0;         
                num_coins = 6; 
                coins[0] = {V3(6,1,0.5), V3(1, -1, 0), monokai.orange}; 
                coins[1] = {V3(-5,-3,0.5), V3(1, -1, 0), monokai.orange}; 
                coins[2] = {V3(17, 0,0.5), V3(1, -1, 0), monokai.orange};
                coins[3] = {V3(30, 0.5,0.5), V3(1, -1, 0), monokai.orange};   
                coins[4] = {V3(39, -3.5,0.5), V3(1, -1, 0), monokai.orange};  
                coins[5] = {V3(48.5, -2.5,0.5), V3(1, -1, 0), monokai.orange} ;              
                tweaks.lose = false;              
            }
        }

        //win screen 
        if(tweaks.win){
            textColor = V3(0,0,0); 
            basic_text(NULL, "you win", 350, 100, 0, textColor.r, textColor.g, textColor.b, 1, 90);
            //basic_text(NULL, "your score is"+points, 350, 200, 0, textColor.r, textColor.g, textColor.b, 1, 90);
            basic_text(NULL, "press [q] to exit", 50, 400, 0, textColor.r, textColor.g, textColor.b, 1, 50);
            basic_text(NULL, "press [r] to retry", 470, 400, 0, textColor.r, textColor.g, textColor.b, 1, 50);
            if(input.key_pressed['R']){
                character.position = V3(-10,2,0.5);
                scroll.origin.x = character.position.x; 
                health = 5; points = 0; 
                num_coins = 6; 
                coins[0] = {V3(6,1,0.5), V3(1, -1, 0), monokai.orange}; 
                coins[1] = {V3(-5,-3,0.5), V3(1, -1, 0), monokai.orange}; 
                coins[2] = {V3(17, 0,0.5), V3(1, -1, 0), monokai.orange};
                coins[3] = {V3(30, 0.5,0.5), V3(1, -1, 0), monokai.orange};   
                coins[4] = {V3(39, -3.5,0.5), V3(1, -1, 0), monokai.orange};  
                coins[5] = {V3(48.5, -2.5,0.5), V3(1, -1, 0), monokai.orange} ;                 
                tweaks.win = false;              
            }
        }

        t += 1. / 60.;       
    }
}

int main() {
    final_proj();
    return 0;
}
