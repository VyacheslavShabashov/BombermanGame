#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

#define row 17
#define col 33
#define max_string_length 32
#define max_bomb 32
#define max_user 32

char map[row][col];
int level = 1;
int hole_row = 1;
int hole_col = 1;
int enemy_count = 2;
int point = 0;
int player_row = 1;
int player_col = 1;
int game_state = 1;
int match_state = 1;
char last_move = 'd';
int total_bomb = 0;
int fps = 100;

HANDLE _output_handle;

void print_at_xy(int x, int y, char *val);

void hidecursor(){
   _output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
   CONSOLE_CURSOR_INFO info;
   info.dwSize = 100;
   info.bVisible = FALSE;
   SetConsoleCursorInfo(_output_handle, &info);
}

void view_highscore(){
	char name[max_user][50];
	char point[max_user][50];
	char line[10][100];
	
	FILE *fp = fopen("save.txt", "r");
	
	int x=0;
    while (1) {
    	if(feof(fp)) break;
		fscanf(fp, "%[^\n]", line[x]); 
		fgetc(fp);
		
		char *p = strtok(line[x], "#");
	
		int z=0;
	    while(p != NULL){
	    	if(z%2 == 0) strcpy(name[x], p);
	    	if(z%2 == 1) strcpy(point[x], p);
	        p = strtok (NULL, "#");
	        z++;
		}
		x++;
    }
	printf("+-----------------------------------+--------+\n");
	printf("| Name                              | Point  |\n");
	printf("+-----------------------------------+--------+\n");
	for(int i=0; i<x-1; i++) printf("+ %-33s | %-6s |\n", name[i], point[i]);
	printf("+-----------------------------------+--------+\n");
	system("pause");
}

void print_map(){
//	char buffer[50] = {0};
//	sprintf(buffer, "LEVEL : %d\n", level);
	printf("LEVEL : %d\n", level);
//	print_at_xy(0, 0, buffer);
	for(int i=0; i<row; i++){
		for(int j=0; j<col; j++){
			printf("%c", map[i][j]);
		}
		printf("\n");
	}
	printf("POINT : %d\n", point);
}

struct enemy{
	int x;
	int y;
	struct enemy *next;
	struct enemy *prev;
}*head = NULL, *tail = NULL;

void popHead(){
	struct enemy *curr = head;
	if(curr == tail){	//data is only 1
		curr = head = tail = NULL;
		free(curr);
	}
	else{
		head = head->next;
		free(curr);
		head->prev = NULL;
	}
}

void popTail(){
	struct enemy *curr = tail;
	if(curr == tail){	//data is only 1
		curr = head = tail = NULL;
		free(curr);
	}
	else{
		tail = tail->prev;
		free(curr);
		tail->next = NULL;
	}
}

void enemy_push_tail(int x, int y){
	struct enemy *curr = (struct enemy *)malloc(sizeof(struct enemy));
	
	curr->x = x;
	curr->y = y;
	
	curr->next = NULL;
	
	if(head == NULL){
		head = tail = curr;
		head->prev = NULL;
	}
	else{
		tail->next = curr;
		curr->prev = tail;
		tail = curr;
	}
}

void delete_enemy(int x, int y){
	struct enemy *curr = head;
	struct enemy *temp = (struct enemy *)malloc(sizeof(struct enemy));
	if(head->x == x && head->y == y){
		popHead();
	}
	else if(tail->x == x && tail->y == y){
		popTail();
	}
	else{
		while(curr->next->x != x && curr->next->y != y){
			curr = curr->next;
		}
		temp = curr->next;
		curr->next = temp->next;
		temp->next->prev = curr;
		free(temp);
	}
}

struct bomb{
	int x;
	int y;
	int time;
	struct bomb *next;
	struct bomb *prev;
}*head_ = NULL, *tail_ = NULL;

void bomb_popHead(){
	struct bomb *curr = head_;
	if(curr == tail_){	//data is only 1
		curr = head_ = tail_ = NULL;
		free(curr);
	}
	else{
		head_ = head_->next;
		free(curr);
		head_->prev = NULL;
	}
}

void bomb_popTail(){
	struct bomb *curr = tail_;
	if(curr == tail_){	//data is only 1
		curr = head_ = tail_ = NULL;
		free(curr);
	}
	else{
		tail_ = tail_->prev;
		free(curr);
		tail_->next = NULL;
	}
}

void bomb_push_tail(int x, int y, int time){
	struct bomb *curr = (struct bomb *)malloc(sizeof(struct bomb));
	
	curr->x = x;
	curr->y = y;
	curr->time = time;
	
	curr->next = NULL;
	
	if(head_ == NULL){
		head_ = tail_ = curr;
		head_->prev = NULL;
	}
	else{
		tail_->next = curr;
		curr->prev = tail_;
		tail_ = curr;
	}
}

void delete_bomb(int x, int y){
	struct bomb *curr = head_;
	struct bomb *temp = (struct bomb *)malloc(sizeof(struct bomb));
	if(head_->x == x && head_->y == y){
		bomb_popHead();
	}
	else if(tail_->x == x && tail_->y == y){
		bomb_popTail();
	}
	else{
		while(curr->next->x != x && curr->next->y != y){
			curr = curr->next;
		}
		temp = curr->next;
		curr->next = temp->next;
		temp->next->prev = curr;
		free(temp);
	}
}

void generate_hole(){
	while(1){
		int random_row = rand()%row;	
		int random_col = rand()%col;
		if(map[random_row][random_col] == 'H'){
			hole_row = random_row;
			hole_col = random_col;
			break;
		}
	}
}

void generate_wall(){
	int random_number = rand() % 10;
	int wall_count = random_number + (level * 10);
	printf("%d\n", random_number);
	for(int x=0; x<wall_count; x++){
		while(1){
			int random_row = rand()%row;	
			int random_col = rand()%col;
			if(map[random_row][random_col] != '#' &&
				map[random_row][random_col] != '@' &&
				map[random_row][random_col] != '-' &&
				map[random_row][random_col] != '|' &&
				map[random_row][random_col] != 'H'){
				map[random_row][random_col] = 'H';
				break;
			}
		}
	}
}

void generate_enemy(){
	for(int x=0; x<enemy_count; x++){	
		while(1){
			int random_row = rand()%row;	
			int random_col = rand()%col;
			if(map[random_row][random_col] != '#' &&
				map[random_row][random_col] != '@' &&
				map[random_row][random_col] != '-' &&
				map[random_row][random_col] != '|' &&
				map[random_row][random_col] != 'H'){
				map[random_row][random_col] = 'X';
				enemy_push_tail(random_row, random_col);
				break;
			}
		}
	}
}

void generate_player(){
	player_row = 1;
	player_col = 1;
	map[player_row][player_col] = '@';
}

char get_input(){
	char ch=0;
	if(kbhit()){
		ch = getch();
	}
	return ch;
}

void update_player(char ch){
	if(ch == 'd' && map[player_row][player_col + 1] != '#' && map[player_row][player_col + 1] != '-' &&
	map[player_row][player_col+1] != '|' && map[player_row][player_col+1] != 'H' 
	&& map[player_row][player_col+1] != '*'){
		last_move = 'd';
		map[player_row][player_col] = ' ';
		player_col++;
		map[player_row][player_col] = '@';
	}
	else if(ch == 's' && map[player_row+1][player_col] != '#' && map[player_row+1][player_col] != '-' &&
	map[player_row+1][player_col] != '|' && map[player_row+1][player_col] != 'H' 
	&& map[player_row+1][player_col] != '*'){
		last_move = 's';
		map[player_row][player_col] = ' ';
		player_row++;
		map[player_row][player_col] = '@';
	}
	else if(ch == 'a' && map[player_row][player_col-1] != '#' && map[player_row][player_col-1] != '-' &&
	map[player_row][player_col-1] != '|' && map[player_row][player_col-1] != 'H'
	&& map[player_row][player_col-1] != '*'){
		last_move = 'a';
		map[player_row][player_col] = ' ';
		player_col--;
		map[player_row][player_col] = '@';
	}
	else if(ch == 'w' && map[player_row-1][player_col] != '#' && map[player_row-1][player_col] != '-' &&
	map[player_row-1][player_col] != '|' && map[player_row-1][player_col] != 'H' 
	&& map[player_row-1][player_col] != '*'){
		last_move = 'w';
		map[player_row][player_col] = ' ';
		player_row--;
		map[player_row][player_col] = '@';
	}
	else if(ch == ' '){
		if(last_move == 'd' && map[player_row][player_col + 1] != '#' && map[player_row][player_col + 1] != '-' &&
		map[player_row][player_col+1] != '|' && map[player_row][player_col+1] != 'H' 
		&& map[player_row][player_col+1] != '*'){
			map[player_row][player_col + 1] = '*';		
			bomb_push_tail(player_row, player_col+1, 2000/fps);
		}
		else if(last_move == 's' && map[player_row+1][player_col] != '#' && map[player_row+1][player_col] != '-' &&
		map[player_row+1][player_col] != '|' && map[player_row+1][player_col] != 'H' 
		&& map[player_row+1][player_col] != '*'){
			map[player_row+1][player_col] = '*';
			bomb_push_tail(player_row+1, player_col, 2000/fps);
		}
		else if(last_move == 'a' && map[player_row][player_col-1] != '#' && map[player_row][player_col-1] != '-' &&
		map[player_row][player_col-1] != '|' && map[player_row][player_col-1] != 'H'
		&& map[player_row][player_col-1] != '*'){
			map[player_row][player_col-1] = '*';		
			bomb_push_tail(player_row, player_col-1, 2000/fps);
		}
		else if(last_move == 'w' && map[player_row-1][player_col] != '#' && map[player_row-1][player_col] != '-' &&
		map[player_row-1][player_col] != '|' && map[player_row-1][player_col] != 'H' 
		&& map[player_row-1][player_col] != '*'){
			map[player_row-1][player_col] = '*';
			bomb_push_tail(player_row-1, player_col, 2000/fps);
		}
		total_bomb++;
	}
	
	if(player_row == hole_row && player_col == hole_col){
		game_state = 0;
		level = level + 1;
	}
	
	if (ch == 27){
		do{
			system("cls");
			int menu_esc;
			printf("1. Back\n");
			printf("2. View Highscores\n");
			printf("3. Exit\n");
			printf(">> ");
			scanf("%d", &menu_esc);
			if(menu_esc == 1) break;
			else if(menu_esc == 2) view_highscore();
			else if(menu_esc == 3){
				game_state = 0;
				match_state = 0;
				break;
			} 
		}while(1);
	}
}

void enemy_move(){
	struct enemy *curr = head;
	while(curr){ // curr != NULL
		int random_move = rand() % 4;
		
		if(random_move == 0 && map[curr->x][curr->y + 1] != '#' && map[curr->x][curr->y + 1] != '-' &&
		map[curr->x][curr->y + 1] != '|' && map[curr->x][curr->y + 1] != 'H' && map[curr->x][curr->y + 1] != '*'){
			if(map[curr->x][curr->y+1] == '@') game_state = 0, match_state = 0;
			map[curr->x][curr->y] = ' ';
			curr->y = curr->y + 1;
			map[curr->x][curr->y] = 'X';
		} 
		else if(random_move == 1 && map[curr->x+1][curr->y] != '#' && map[curr->x+1][curr->y] != '-' &&
		map[curr->x+1][curr->y] != '|' && map[curr->x+1][curr->y] != 'H' && map[curr->x+1][curr->y] != '*'){
			if(map[curr->x+1][curr->y] == '@') game_state = 0, match_state = 0;
			map[curr->x][curr->y] = ' ';
			curr->x = curr->x + 1;
			map[curr->x][curr->y] = 'X';
		}
		else if(random_move == 2 && map[curr->x][curr->y-1] != '#' && map[curr->x][curr->y-1] != '-' &&
		map[curr->x][curr->y-1] != '|' && map[curr->x][curr->y-1] != 'H' && map[curr->x][curr->y-1] != '*'){
			if(map[curr->x][curr->y-1] == '@') game_state = 0, match_state = 0;
			map[curr->x][curr->y] = ' ';
			curr->y = curr->y - 1;
			map[curr->x][curr->y] = 'X';
		}
		else if(random_move == 3 && map[curr->x-1][curr->y] != '#' && map[curr->x-1][curr->y] != '-' &&
		map[curr->x-1][curr->y] != '|' && map[curr->x-1][curr->y] != 'H' && map[curr->x-1][curr->y] != '*'){
			if(map[curr->x-1][curr->y] == '@') game_state = 0, match_state = 0;
			map[curr->x][curr->y] = ' ';
			curr->x = curr->x - 1;
			map[curr->x][curr->y] = 'X';
		}	
		curr = curr->next;
	}
}

void add_user(char user[max_string_length], long score){
	FILE *fp = fopen("save.txt", "a+");
	fprintf(fp, "%s#%d\n", user, score);
}

void create_map_file(){
	FILE *fp = fopen("map.txt", "a+");
	fprintf(fp, "---------------------------------\n");
	fprintf(fp, "|                               |\n");
	fprintf(fp, "|   #   #   #   #   #   #   #   |\n");
	fprintf(fp, "|                               |\n");
	fprintf(fp, "|   #   #   #   #   #   #   #   |\n");		
	fprintf(fp, "|                               |\n");
	fprintf(fp, "|   #   #   #   #   #   #   #   |\n");				
	fprintf(fp, "|                               |\n");
	fprintf(fp, "|   #   #   #   #   #   #   #   |\n");						
	fprintf(fp, "|                               |\n");
	fprintf(fp, "|   #   #   #   #   #   #   #   |\n");		
	fprintf(fp, "|                               |\n");
	fprintf(fp, "|   #   #   #   #   #   #   #   |\n");				
	fprintf(fp, "|                               |\n");
	fprintf(fp, "|   #   #   #   #   #   #   #   |\n");						
	fprintf(fp, "|                               |\n");
	fprintf(fp, "---------------------------------\n");
}

void load_map(){
	FILE *fp = fopen("map.txt", "a+");
	if(! fp ) create_map_file();
	else{
		int x=0;
	    while (1) {
	    	if(feof(fp)) break;
			fscanf(fp, "%[^\n]", map[x]); fgetc(fp);
			x++;
	    }
	}
}

void print_at_xy(int x, int y, char *val){
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(_output_handle, coord);
//	printf("%s", (const char *)val);
	fflush(stdout);
}

void clear_screen(){
    char buffer[50] = {0};
    print_at_xy(0, 0, buffer);
}

void bomb_exploded(int row_, int col_){
	if(map[row_][col_] != '#' && map[row_][col_] != '-' && map[row_][col_] != '|'){
		if(map[row_][col_] == '@') game_state = 0, match_state = 0;
		if(map[row_][col_] == 'H') {
			point = point + 50;
		}
		if(map[row_][col_] == 'X'){
			enemy_count--;
			delete_enemy(row_, col_);
			point = point + 100;
		} 
		map[row_][col_] = ' ';
		if(row_ == hole_row && col_ == hole_col) map[row_][col_] = 'O';
	}
}

void detonate_bomb(){
	struct bomb *curr = head_;
	while(curr){ // curr != NULL
		curr->time = curr->time - 1;
		if(curr->time == 0){
			bomb_exploded(curr->x, curr->y); // posisi bomb
			
			bomb_exploded(curr->x+1, curr->y); // bawah
			bomb_exploded(curr->x, curr->y+1); // kanan
			bomb_exploded(curr->x-1, curr->y); // atas
			bomb_exploded(curr->x, curr->y-1); // kiri
			bomb_exploded(curr->x+1, curr->y+1); // kanan bawah
			bomb_exploded(curr->x-1, curr->y+1); // kanan atas
			bomb_exploded(curr->x+1, curr->y-1); // kiri bawah
			bomb_exploded(curr->x-1, curr->y-1); // kiri atas
			total_bomb--;
		}
		curr = curr->next;
	}

}

void init(){
	player_row = 1;
	player_col = 1;
	game_state = 1;
	last_move = 'd';
	total_bomb = 0;
	generate_enemy();
}

int main(void){
	
	hidecursor();
	srand(time(NULL));
	printf("Input FPS: ");
	scanf("%d", &fps);
	
	do{
		load_map();
		init();
		generate_player();
		generate_wall();
		generate_hole();
		
		do{
//			printf("%d %d\n", hole_row, hole_col);
			print_map();
			char ch = get_input();
			update_player(ch);
			enemy_move();
			
			if(game_state == 0) break;
			
			
			Sleep(1000/fps);
			
			detonate_bomb();
		
//			clear_screen(); lebih jernih pake clear screen sebenernya tapi ada bug (nge print buffer di terminal)
			system("@cls || clear");
		}while(true);
		
		if(game_state == 0 && match_state == 0){
			char uname[32];
			printf("Enter Name : ");
			scanf("%s", uname);	
			add_user(uname, point);
			printf("SUCCESS SAVE!\n");
			system("pause");
			break;
		} 
		system("cls");
	}while(true);
	
	
	return 0;
}
