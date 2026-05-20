#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define TOTAL_NUMBER_NUM 30
#define TOTAL_SYMBOL_NUM 16
#define MAX_SYMBOL_LETTER 10
#define STARTING_HAND_NUMBER_NUM 10
#define STARTING_HAND_SYMBOL_NUM 5
#define STARTING_HAND_TOTAL_NUM (STARTING_HAND_NUMBER_NUM + STARTING_HAND_SYMBOL_NUM)
#define PLAYER_NUM 2
#define MAX_DISCARD_NUM (STARTING_HAND_TOTAL_NUM * PLAYER_NUM)

#define COLOR_RESET   "\x1b[0m"
#define COLOR_BOLD    "\x1b[1m"
#define COLOR_CYAN    "\x1b[36m"  // 数字カード用
#define COLOR_YELLOW  "\x1b[33m"  // 記号カード用
#define COLOR_GREEN   "\x1b[32m"  // プレイヤー名用

typedef struct
{
    int number;
    char *symbol;
    bool is_number;
} Card;

typedef struct
{
    Card hand[STARTING_HAND_TOTAL_NUM];
    int hand_num;
} Player;

typedef struct
{
    Player players[PLAYER_NUM];
    Card field_cards[MAX_DISCARD_NUM];
    int field_cards_num;
    int next_player;
    bool is_end;
} Game_State;


// 任意の型の配列をシャッフルする汎用関数
void shuffle(void *base, size_t nmemb, size_t size) {
    if (nmemb > 1 && size > 0) {
        // バイト単位でポインタ演算を行うため char * にキャスト
        char *arr = (char *)base;
        
        for (size_t i = nmemb - 1; i > 0; i--) {
            size_t j = rand() % (i + 1);
            
            if (i != j) {
                // size バイト分だけ要素をスワップする
                for (size_t k = 0; k < size; k++) {
                    char temp = arr[i * size + k];
                    arr[i * size + k] = arr[j * size + k];
                    arr[j * size + k] = temp;
                }
            }
        }
    }
}

// set starting hand in array. 
void set_card(Game_State *game_state){
    int all_number[TOTAL_NUMBER_NUM] = {1,1,1,2,2,2,3,3,3,4,4,4,5,5,5,6,6,6,7,7,7,8,8,8,9,9,9,10,10,10};
    char *all_symbol[TOTAL_SYMBOL_NUM] = {"+1","+1","+1","+1","+2","+2","+2","+2","-1","-1","-1","-1","return","return","return","return"};

    shuffle(all_number,TOTAL_NUMBER_NUM, sizeof(int));
    shuffle(all_symbol,TOTAL_SYMBOL_NUM, sizeof(char *));

    for(int player = 0; player < PLAYER_NUM; player++){
        for(int i = 0; i < STARTING_HAND_NUMBER_NUM; i++){
            Card card = {
                all_number[i + player * STARTING_HAND_NUMBER_NUM],
                "",
                1
            };
            game_state->players[player].hand[i] = card;
        }   
    }

    for(int player = 0; player < PLAYER_NUM; player++){
        for(int i = 0; i < STARTING_HAND_SYMBOL_NUM; i++){
            Card card = {
                -1,
                all_symbol[i + player * STARTING_HAND_SYMBOL_NUM],
                0
            };
            game_state->players[player].hand[i + STARTING_HAND_NUMBER_NUM] = card;
        }   
    }

    Card card = {
        all_number[PLAYER_NUM * STARTING_HAND_NUMBER_NUM],
        "",
        1
    };
    game_state->field_cards[0] = card;
    game_state->field_cards_num = 1;
}

// initailize game state.
void init_game(Game_State *game_state){
    set_card(game_state);
    for(int i = 0; i < PLAYER_NUM; i++) game_state->players[i].hand_num = STARTING_HAND_TOTAL_NUM;
    game_state->is_end = false;
    game_state->next_player = 0;
}

//display each player's hands. for debug 
void display_hand(Game_State *game_state){
    for(int player = 0; player < PLAYER_NUM; player++){
        printf("player%d: ",player);
        for(int i = 0; i < game_state->players[player].hand_num; i++){
            bool is_number = game_state->players[player].hand[i].is_number;
            if(is_number){
                printf("%d ",game_state->players[player].hand[i].number);
            }else{
                printf("%s ",game_state->players[player].hand[i].symbol);
            }  
        }
        printf("\n");
    }
}

void display_card_contents(Card card){
    bool is_number = card.is_number;
    if(is_number){
        printf("%d",card.number);
    }else{
        printf("%s",card.symbol);
    }
}

//Display the specified player's hand.
void display_player_hand(Game_State *game_state, int player){
    printf(COLOR_BOLD COLOR_GREEN "=== Player %d's Hand (Total: %d) ===\n" COLOR_RESET, player, game_state->players[player].hand_num);
    int hand_num = game_state->players[player].hand_num;
    printf("\n");
    for(int i = 0; i < hand_num; i++){
        Card card = game_state->players[player].hand[i];
        Card c = game_state->players[player].hand[i];

        printf(" %2d: ", i);

        if (c.is_number) {
            printf(COLOR_CYAN "[ %2d ]" COLOR_RESET, c.number);
        } else {
            char *disp_symbol = c.symbol;
            if (strcmp(disp_symbol, "return") == 0) {
                disp_symbol = "Re";
            }
            printf(COLOR_YELLOW "[ %2s ]" COLOR_RESET, disp_symbol);
        }

        // 5枚ごとに改行を入れて画面端での折り返し崩れを防ぐ
        if ((i + 1) % 5 == 0 || i == hand_num - 1) {
            printf("\n");
        } else {
            // カード間の区切り線
            printf(" |");
        }
    }
    printf("\n");
}

void display_field(Game_State *game_state){
    printf("[");
    for(int i = 0; i < game_state->field_cards_num; i++){
        display_card_contents(game_state->field_cards[i]);
        printf(" ");
    }
    printf("]\n");
}

int calculate_field_sum(Game_State *game_state){
    int sum = 0;
    for(int i = 0; i < game_state->field_cards_num; i++){
        Card *card = &game_state->field_cards[i];
        if(card->is_number){
            sum+=card->number;
        }else{
            char *symbol = card->symbol;
            if(symbol[0] == '+'){
                sum+=atoi(symbol+1);
            }else if(symbol[0] == '-'){
                sum-=atoi(symbol+1);
            }
        }
    }
    return sum;
}

void put_card_to_field(Game_State *game_state, Card card){
    game_state->field_cards[game_state->field_cards_num] = card;
    game_state->field_cards_num++;
}

int search_card(Game_State *game_state, int player, Card card){
    int index = -1;
    for(int i = 0; i < game_state->players[player].hand_num; i++){
        Card *hand_card = &game_state->players[player].hand[i];
        if(hand_card->is_number == card.is_number){
            if(card.is_number){
                if(hand_card->number == card.number){
                    index = i;
                    break;
                }
            }else{
                if(strcmp(hand_card->symbol, card.symbol) == 0){
                    index = i;
                    break;
                }
            }
        }
    }
    return index;
}

void delete_card(Game_State *game_state, int player, int index){
    for(int i = index + 1; i < game_state->players[player].hand_num; i++){
        game_state->players[player].hand[i-1] = game_state->players[player].hand[i];
    }
    game_state->players[player].hand_num--;
}

void delete_card_by_obj(Game_State *game_state, int player, Card card){
    int index = search_card(game_state,player,card);
    delete_card(game_state,player,index);
}

void put_card(Game_State *game_state,int player){
    Player *player_obj = &game_state->players[player];

    printf("player%d turn\n",player);
    printf("現在の場↓ \n");
    display_field(game_state);   

    printf("あなたの手札↓ \n");
    display_player_hand(game_state,player);

    int input_num = -1,input_symbol = -1;
    Card card_num,card_symbol;

    bool is_correct_range;
    do{
        printf("場に出す数字カードを選択してください");
        if (scanf("%d", &input_num) != 1) {
            while (getchar() != '\n'); // バッファを空にする
            continue;
        }
        is_correct_range = (0 <= input_num && input_num < player_obj->hand_num);
    }while(!is_correct_range || !player_obj->hand[input_num].is_number);
    card_num = player_obj->hand[input_num];

    printf("%dが選択されました\n",card_num.number);

    do{
        printf("場に出す記号カードを選択してください(出さない場合は-1)");
        if (scanf("%d", &input_symbol) != 1) {
            while (getchar() != '\n'); 
            continue;
        }
        
        if (input_symbol == -1) {
            break; 
        }

        is_correct_range = (0 <= input_symbol && input_symbol < player_obj->hand_num);
    }while(!is_correct_range || player_obj->hand[input_symbol].is_number);
    card_symbol = player_obj->hand[input_symbol];

    if(input_symbol != -1){
        printf("%sが選択されました\n",card_symbol.symbol);
    }else{
        printf("記号カード未使用");
    }

    put_card_to_field(game_state,card_num);
    delete_card_by_obj(game_state,player,card_num);
    if(input_symbol != -1){
        put_card_to_field(game_state,card_symbol);
        delete_card_by_obj(game_state,player,card_symbol);
    }

    printf("今の合計は %d \n",calculate_field_sum(game_state));
}

void update_state(Game_State *game_state,int player){

    if(game_state->players[player].hand_num == 0){
        game_state->is_end = true;
        printf("player%d win",player);
        return;
    }

    int sum = calculate_field_sum(game_state);
    game_state->next_player = (game_state->next_player + 1) % PLAYER_NUM;
    if(sum == 15){
        game_state->next_player = player;
        game_state->field_cards_num = 0;
    }

    if(game_state->field_cards[game_state->field_cards_num-1].symbol == "return"){
        game_state->next_player = player;
    }
    
    if(sum > 15){
        game_state->field_cards_num = 0;
    }
}

int main(void){
    //srand((unsigned)time(NULL));
    srand(0);
    Game_State game_state;
    init_game(&game_state);

    game_state.next_player = 0;

    while(!game_state.is_end){
        put_card(&game_state,game_state.next_player);
        update_state(&game_state,game_state.next_player);
        printf("\n\n");
    }

    printf("\n");
    return 0;
}