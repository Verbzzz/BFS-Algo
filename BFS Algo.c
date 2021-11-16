
#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define PN 10 //Size of Propabilities table

//Structs for List
typedef struct list_node //represents every node
{
	int vertex;
	struct list_node* next;
}list_node;

typedef struct adj_list //represents the adjacency list
{
	struct list_node* head;
}adj_list;

typedef struct Graph //represents a gragh
{
	int V;
	struct adj_list* array;
}Graph;

//Struct for Queue
typedef struct queue //struct for Queue
{
	int* items;     // array to store queue elements
	int maxsize;    // maximum capacity of the queue
	int front;      // front points to the front element in the queue (if any)
	int rear;       // rear points to the last element in the queue
	int size;
}queue;


// Functions declarations!!

// Creating a new List Node
list_node* new_list_node(int dest);
list_node* freeList(list_node* head);

// Functions for Queue
queue* createQueue(int size);
void enqueue(queue* q, int value);
int dequeue(struct queue* q);
int isEmpty(struct queue* q);
void freeQueue(queue* q);
int size(queue* pt);


//Building a Graph
Graph* createGraph(int V);
void addEdge(Graph* graph, int src, int dest);
void freeGraph(Graph* g);

//Functions of the assignment
Graph* build_random_graph(int v, float p);
int Is_Isolated(Graph* graph);
int connectivity(Graph* graph);
int diameter(Graph* graph);

//Additional functions
void BFS_Check4ConnectAndDiam(Graph* graph, int startVertex, int** visited, int* temp_diam);
void writeToCSV(char* name, float probs[PN], float means[PN]);
void createTable(char* name, int(*fn)(Graph* g), int v, int runs, float thresh);
void createTable4Diam(char* name, int(*fn)(Graph* g), int v, int runs, float thresh);
void makeProbsArray(float arr[PN], float p);



int main()
{
	srand(time(0));
	int V = 1000; // Amount of Vertices
	int runs = 500; // Amount of runs
	createTable("connectivity.csv", connectivity, V, runs, (float)log(V) / V);
	createTable4Diam("diameter.csv", diameter, V, runs, sqrt(2 * (float)log(V) / V));
	createTable("Is_Isolated.csv", Is_Isolated, V, runs, (float)log(V) / V);

	return 0;
}

Graph* build_random_graph(int v, float p) //Builds a graph with erdos-renyi model.
{
	int i, j;
	float random_num;
	Graph* graph = createGraph(v);

	for (i = 0; i < v; i++)
		for (j = i + 1; j < v; j++)
			if (i != j && ((float)rand() / RAND_MAX) <= p)
				addEdge(graph, i, j);
	return graph;
}

int Is_Isolated(Graph* graph)//Checks if the graph contains an Isolated vertex.
{
	int i;
	for (i = 0; i < graph->V; ++i)
	{
		if (graph->array[i].head == NULL)
			return 1;
	}
	return 0;
}

int connectivity(Graph* graph)//Checks if the graph is Connected- meaning if each vertes has a path to all the other Vertices.
{
	int v = graph->V;
	int i;
	int temp_diam = 0;
	int* visited = (int*)calloc(v, sizeof(int));

	BFS_Check4ConnectAndDiam(graph, 0, visited, &temp_diam); //V+E

	for (i = 0; i < v; i++)
	{
		if (visited[i] == 0)
		{
			free(visited);
			return 0;
		}
	}
	free(visited);
	return 1;
}

int diameter(Graph* graph) //Finds the Diameter of the graph.
{
	if (!connectivity(graph))return INT_MAX;
	int v = graph->V;
	int i, j;
	int temp_diam = 0;
	int diam = 0;

	int* visited = (int*)calloc(v, sizeof(int));

	for (i = 0; i < v; i++)
	{
		BFS_Check4ConnectAndDiam(graph, i, visited, &temp_diam); //V+E

		if (temp_diam > diam)
			diam = temp_diam;

		for (j = 0; j < v; j++) { visited[j] = 0; } //new visited
	}
	free(visited);
	return diam;
}

void BFS_Check4ConnectAndDiam(Graph* graph, int startVertex, int** visited, int* temp_diam)//Collects the max distance of the min distances from a specific vertex + checks if the specific vertex has path to all other vertices.
{
	int v = graph->V;
	queue* q = createQueue(v);
	int i, adjVertex;
	int max_num = 0;

	int* distance = (int*)calloc(v, sizeof(int));

	visited[startVertex] = 1;
	enqueue(q, startVertex);

	while (!isEmpty(q))
	{
		int currentVertex = dequeue(q);

		list_node* temp = graph->array[currentVertex].head;

		while (temp)
		{
			adjVertex = temp->vertex;
			if (visited[adjVertex] == 0)
			{
				visited[adjVertex] = 1;
				enqueue(q, adjVertex);
				distance[adjVertex] = distance[currentVertex] + 1;
				if (distance[adjVertex] > max_num)
					max_num = distance[adjVertex];
			}
			temp = temp->next;
		}
	}
	(*temp_diam) = max_num;
	free(distance);
	freeQueue(q);
}

void makeProbsArray(float arr[PN], float p) //Algorithem to achieve an array with 5 elements lower then Threshold and 5 greater then the Threshold.
{
	int i;
	for (int i = 0; i < PN; i++)
		arr[i] = (i + 1) * 2 * p / (10.1);
}

void writeToCSV(char* name, float probs[PN], float means[PN]) //Writes the requested table of probabilities to a CSV file
{
	FILE* fp = fopen(name, "w");
	int i;
	for (i = 0; i < 9; i++) fprintf(fp, "%f,", probs[i]);
	fprintf(fp, "%f\n", probs[i]);
	for (i = 0; i < 9; i++) fprintf(fp, "%f,", means[i]);
	fprintf(fp, "%f\n", means[i]);
	fclose(fp);
}

void createTable(char* name, int(*fn)(Graph* g), int v, int runs, float thresh) //Creates the table that will be written to the CSV file.
{
	float meanArr[PN] = { 0 }; // resetting the means array with zeros
	float probsArr[PN];
	int i, j; //indexs
	makeProbsArray(probsArr, thresh);
	for (i = 0; i < PN; i++) {
		for (j = 0; j < runs; j++) {
			Graph* graph = build_random_graph(v, probsArr[i]);
			meanArr[i] += fn(graph);
			freeGraph(graph);
		}
		meanArr[i] /= runs;
	}
	writeToCSV(name, probsArr, meanArr);
}

void createTable4Diam(char* name, int(*fn)(Graph* g), int v, int runs, float thresh) //Creates the table that will be written to the CSV file.
{
	float meanArr[PN] = { 0 }; // resetting the means array with zeros
	float probsArr[PN];
	int i, j; //indexs
	makeProbsArray(probsArr, thresh);
	for (i = 0; i < PN; i++) {
		for (j = 0; j < runs; j++) {
			Graph* graph = build_random_graph(v, probsArr[i]);
			if (fn(graph) <= 2)
			{
				meanArr[i] += 1;
			}
			freeGraph(graph);
		}
		meanArr[i] /= runs;
	}
	writeToCSV(name, probsArr, meanArr);
}

//Supporting Functions
list_node* new_list_node(int dest) // A function to create a new node
{
	list_node* new_node = (list_node*)malloc(sizeof(list_node));
	new_node->vertex = dest;
	new_node->next = NULL;
	return new_node;
}

list_node* freeList(list_node* head)
{
	list_node* next;
	if (head == NULL) return NULL;
	next = head->next;
	free(head);
	return freeList(next);
}

Graph* createGraph(int V) // A function that creates a graph of V vertices (EMPTY)
{
	int i;
	Graph* graph = (Graph*)malloc(sizeof(Graph));
	graph->V = V;
	graph->array = (adj_list*)malloc(V * sizeof(adj_list)); // Create an array of adjacency lists in size of V
	for (i = 0; i < V; ++i) // Initialize the heads of the list by putting NULL
	{
		graph->array[i].head = NULL;
	}
	return graph;
}

void addEdge(Graph* graph, int src, int dest) //adding edges for every connection. for exmple: 4(src)<->2(des)
{
	list_node* new_node = new_list_node(dest);
	new_node->next = graph->array[src].head;
	graph->array[src].head = new_node;

	new_node = new_list_node(src);
	new_node->next = graph->array[dest].head;
	graph->array[dest].head = new_node;
}

void freeGraph(Graph* g)
{
	int i;
	for (i = 0; i < g->V; i++) freeList(g->array[i].head);
	free(g->array);
}

// Create a queue
queue* createQueue(int size)
{
	queue* pt = NULL;
	pt = (queue*)malloc(sizeof(queue));
	pt->items = (int*)malloc(size * sizeof(int));
	pt->maxsize = size;
	pt->front = 0;
	pt->rear = -1;
	pt->size = 0;
	return pt;
}

void freeQueue(queue* q) {
	queue* next, * node = q->items;
	next = q->front;
	free(node);
}

// Adding elements into queue
void enqueue(queue* q, int value)
{
	if (size(q) == q->maxsize)
	{
		printf("Overflow\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}
	q->rear = (q->rear + 1) % q->maxsize;
	q->items[q->rear] = value;
	q->size++;
}

// Removing elements from queue
int dequeue(queue* q) {
	int item;
	if (isEmpty(q))    // front == rear
	{
		printf("Underflow\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}
	item = q->items[q->front];

	q->front = (q->front + 1) % q->maxsize;
	q->size--;
	return item;
}

int size(queue* pt) {
	return pt->size;
}

// Check if the queue is empty
int isEmpty(queue* q) {
	return !size(q);
}


