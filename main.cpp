#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

// =========================================================
// 0. Essential Helper Functions
// =========================================================

void manual_swap(string& a, string& b) {
    string temp = a;
    a = b;
    b = temp;
}

// =========================================================
// 1. Core Graph Data Structures (Node, Linkedlist, Vertex)
// =========================================================

struct Node {
    string destination;
    int weight;
    Node* next;
};

class Linkedlist {
public:
    Node* head;
    Linkedlist() : head(nullptr) {}
    ~Linkedlist() {
        Node* current = head;
        Node* next;
        while (current != nullptr) {
            next = current->next;
            delete current;
            current = next;
        }
    }
    // Adds an edge with a fixed weight of 1 (for station count)
    void addLast(string dest) {
        Node* newNode = new Node;
        newNode->destination = dest;
        newNode->weight = 1;
        newNode->next = nullptr;
        if (head == nullptr) { head = newNode; return; }
        Node* temp = head;
        while (temp->next != nullptr) { temp = temp->next; }
        temp->next = newNode;
    }
    void display() {
        Node* temp = head;
        while (temp != nullptr) {
            cout << "(" << temp->destination << ") -> ";
            temp = temp->next;
        }
        cout << "NULL" << endl;
    }
    // Check function (used for the Visited Set in BFS)
    bool contains(string item) {
        Node* temp = head;
        while (temp != nullptr) {
            if (temp->destination == item) {
                return true;
            }
            temp = temp->next;
        }
        return false;
    }
};

class Vertex {
public:
    string id; // Station name
    Linkedlist edges;
    Vertex(string id) : id(id) {}
    ~Vertex() {}
};

// =========================================================
// 2. User's Provided Queue (StationQueue)
// =========================================================

class StationQueue {
private:
    static const int MAX = 100;
    string arr[MAX];
    int top;

public:
    StationQueue() { top = 0; }
    string peek() {
        if (isEmpty()) { throw runtime_error("Queue is empty!"); }
        return arr[0];
    }
    string dequeue() {
        if (isEmpty()) { throw runtime_error("Queue is empty!"); }
        string dequeuedValue = arr[0];
        // Shift elements forward
        for (int i = 1; i < top; i++) { arr[i - 1] = arr[i]; }
        top--;
        return dequeuedValue;
    }
    void enqueue(string value) {
        if (top == MAX) { cout << "Queue is full!" << endl; return; }
        arr[top++] = value;
    }
    bool isEmpty() { return top == 0; }
    int getCount() { return top; }
};

// =========================================================
// 3. CustomMap (HashTable)
// =========================================================

struct MapNode {
    string key;
    int count_value;    // Station count / distance
    string pred_value;  // Predecessor station
    MapNode* next;
};

class CustomMap {
private:
    static const int CAPACITY = 100;
    MapNode* table[CAPACITY];
    int size;

    unsigned int hashFunction(const string& key) {
        unsigned int hash = 0;
        for (size_t i = 0; i < key.length(); i++) {
            hash = (hash * 31) + key[i];
        }
        return hash % CAPACITY;
    }

public:
    CustomMap() : size(0) {
        for (int i = 0; i < CAPACITY; i++) { table[i] = nullptr; }
    }
    // Gets node or creates a new one if not found
    MapNode* getOrCreate(const string& key) {
        unsigned int index = hashFunction(key);
        MapNode* current = table[index];
        while (current != nullptr) {
            if (current->key == key) { return current; }
            current = current->next;
        }
        MapNode* newNode = new MapNode;
        newNode->key = key;
        newNode->count_value = 0;
        newNode->pred_value = "";
        newNode->next = table[index];
        table[index] = newNode;
        size++;
        return newNode;
    }
    // Finds an existing node
    MapNode* find(const string& key) {
        unsigned int index = hashFunction(key);
        MapNode* current = table[index];
        while (current != nullptr) {
            if (current->key == key) { return current; }
            current = current->next;
        }
        return nullptr;
    }

    // Destructor to free memory
    ~CustomMap() {
        for (int i = 0; i < CAPACITY; ++i) {
            MapNode* current = table[i];
            while (current != nullptr) {
                MapNode* next = current->next;
                delete current;
                current = next;
            }
        }
    }
};

// =========================================================
// 4. Graph Class and Breadth-First Search (BFS)
// =========================================================

class Graph {
private:
    Vertex* vertices[100];
    int vertexCount;

    int getVertexIndex(string id) {
        for (int i = 0; i < vertexCount; i++) {
            if (vertices[i]->id == id) { return i; }
        }
        return -1;
    }

    // Function to calculate ticket price based on user-defined tiers
    int calculateTicketPrice(int stationsCount) {
        if (stationsCount <= 9) {
            return 8; // 1-9 stations
        } else if (stationsCount <= 16) {
            return 10; // 10-16 stations
        } else if (stationsCount <= 23) {
            return 15; // 17-23 stations
        } else {
            return 20; // more than 23 stations
        }
    }

public:
    Graph() : vertexCount(0) {}
    ~Graph() {
        for (int i = 0; i < vertexCount; i++) { delete vertices[i]; }
    }

    // Public getter for vertexCount (fixes the 'private' error)
    int getVertexCount() const {
        return vertexCount;
    }

    void addVertex(string id) {
        if (getVertexIndex(id) != -1) { return; }
        if (vertexCount < 100) { vertices[vertexCount++] = new Vertex(id); }
    }

    // Adds a bi-directional edge
    void addEdge(string src, string dest) {
        int srcIndex = getVertexIndex(src);
        int destIndex = getVertexIndex(dest);

        if (srcIndex != -1 && destIndex != -1) {
            vertices[srcIndex]->edges.addLast(dest);
            vertices[destIndex]->edges.addLast(src);
        }
    }

    void displayGraph() {
        for (int i = 0; i < vertexCount; i++) {
            cout << "Station: " << vertices[i]->id << " -> Connections: ";
            vertices[i]->edges.display();
        }
    }

    // Implements BFS to find the shortest path by number of stations
    void findShortestPath(string startStation, string endStation) {
        if (getVertexIndex(startStation) == -1 || getVertexIndex(endStation) == -1) {
            cout << "Error: Start or end station not found in the network.\n";
            return;
        }

        CustomMap map;
        const int INF = 1000000;

        Linkedlist visited;

        // 1. Initialization
        for (int i = 0; i < vertexCount; i++) {
            MapNode* node = map.getOrCreate(vertices[i]->id);
            node->count_value = INF;
            node->pred_value = "";
        }

        MapNode* startNode = map.find(startStation);
        if (startNode) startNode->count_value = 0;

        // 2. BFS Traversal
        StationQueue q;
        q.enqueue(startStation);
        visited.addLast(startStation);

        while (!q.isEmpty()) {
            string current_node_id;
            try { current_node_id = q.dequeue(); }
            catch (const runtime_error& e) { break; }

            if (current_node_id == endStation) { break; }

            int current_node_index = getVertexIndex(current_node_id);
            MapNode* current_map_node = map.find(current_node_id);

            if (current_node_index != -1 && current_map_node != nullptr) {
                Node* neighbor = vertices[current_node_index]->edges.head;

                while (neighbor != nullptr) {
                    string neighbor_id = neighbor->destination;

                    if (!visited.contains(neighbor_id)) {
                        visited.addLast(neighbor_id);
                        q.enqueue(neighbor_id);

                        MapNode* neighbor_map_node = map.find(neighbor_id);
                        if (neighbor_map_node) {
                            // Update: Distance = Current Distance + 1
                            neighbor_map_node->count_value = current_map_node->count_value + 1;
                            neighbor_map_node->pred_value = current_node_id;
                        }
                    }
                    neighbor = neighbor->next;
                }
            }
        }

        // 3. Output Results and Path Reconstruction
        MapNode* end_map_node = map.find(endStation);
        if (end_map_node == nullptr || end_map_node->count_value == INF) {
            cout << "\nCannot reach " << endStation << " from " << startStation << ".\n";
        } else {
            int stations_count = end_map_node->count_value;
            int ticket_price = calculateTicketPrice(stations_count); // Calculate Price

            cout << "\n--- Shortest Metro Route (Stations Count) ---\n";
            cout << "Total number of stations to pass: " << stations_count << " stations.\n";
            cout << "----------------------------------------------\n";
            cout << "Price for this trip: " << ticket_price << " EGP.\n"; // Print Price
            cout << "----------------------------------------------\n";

            string path_array[100];
            int path_size = 0;
            string current = endStation;

            // Reconstruct path by traversing predecessors
            while (current != startStation) {
                path_array[path_size++] = current;
                MapNode* pred_node = map.find(current);
                if (pred_node == nullptr || pred_node->pred_value == "") break;
                current = pred_node->pred_value;
            }
            path_array[path_size++] = startStation;

            // Reverse the path manually
            for (int i = 0; i < path_size / 2; i++) {
                manual_swap(path_array[i], path_array[path_size - 1 - i]);
            }

            cout << "Route: ";
            for (int i = 0; i < path_size; ++i) {
                cout << path_array[i];
                if (i < path_size - 1) { cout << " -> "; }
            }
            cout << "\n----------------------------\n";
        }
    }
};

// =========================================================
// 5. Main Function (Cairo Metro Stations and Menu)
// =========================================================

void displayOrders() {
    cout << "\n--- Metro Network Analyzer (Shortest Stations) ---\n";
    cout << "0. Exit\n";
    cout << "1. Add Station\n";
    cout << "2. Add Line Segment (Edge)\n";
    cout << "3. Find Shortest Path (By Stations Count & Price)\n";
    cout << "4. Display All Connections\n";
}

// Helper function to add all stations and edges for a single continuous line segment
void add_line(Graph& g, const string stations[], int count) {
    for (int i = 0; i < count; ++i) {
        g.addVertex(stations[i]);
    }
    for (int i = 0; i < count - 1; ++i) {
        g.addEdge(stations[i], stations[i+1]);
    }
}

int main() {
    Graph metroGraph;

    cout << "Setting up initial metro network...\n";

    // Line 1 (Helwan - New El Marg)
    const string L1[] = {
        "Helwan", "Ain Helwan", "Helwan University", "Wadi Hof", "Hadaiq Helwan", "El Ma'asara",
        "Tora El Asmant", "Kotsika", "Tora El Balad", "Thakanat El Maadi", "El Maadi",
        "Hadaiq El Maadi", "Dar El Salam", "El Zahraa", "Mar Girgis", "El Malek El Saleh",
        "Al-Sayeda Zeinab", "Saad Zaghloul", "Sadat", "Gamal Abdel Nasser", "Orabi",
        "El Shohadaa", "Ghamra", "El Demerdash", "Manshiet El Sadr", "Kobri El Qobba",
        "Hammamat El Qobba", "Saray El Qobba", "Hadaiq El Zeitoun", "Helmiet El Zeitoun",
        "El Matareyya", "Ain Shams", "Ezbet El Nakhl", "El Marg", "New El Marg"
    };
    add_line(metroGraph, L1, sizeof(L1)/sizeof(L1[0]));

    // Line 2 (Shubra El Kheima - El Mounib)
    const string L2[] = {
        "Shubra El Kheima", "Kolleyyet El Zeraa", "Rod El Farag", "El Khalafawy", "St. Teresa",
        "Masarra", "El Shohadaa", "Attaba", "Mohamed Naguib", "Sadat", "Opera",
        "Dokki", "El Bohouth", "Cairo University", "Faisal", "Giza",
        "Om El Masryeen", "El Mounib"
    };
    add_line(metroGraph, L2, sizeof(L2)/sizeof(L2[0]));

    // Line 3 - Eastern Section (Adly Mansour - Kit Kat)
    const string L3_East[] = {
        "Adly Mansour", "El Haykesteb", "Omar Ibn El Khattab", "Qibaa", "Hisham Barakat",
        "El Nozha", "Nady El Shams", "Alf Masken", "Heliopolis", "Haroun",
        "El Ahram", "Kolleayet El Banat", "El Istad", "El Ma'arad", "Abassiya",
        "Abdou Pasha", "El Geish", "Bab El Shaaria", "Attaba", "Gamal Abdel Nasser",
        "Maspero", "Safaa Hegazy", "Kit Kat"
    };
    add_line(metroGraph, L3_East, sizeof(L3_East)/sizeof(L3_East[0]));

    // Line 3 - Branch 1 (Kit Kat -> Rod El Farag Axis)
    const string L3_Branch1[] = {
        "El Sudan", "Embaba", "El Bohoey", "El Qawmeyya", "Ring Road", "Rod El Farag Axis"
    };
    add_line(metroGraph, L3_Branch1, sizeof(L3_Branch1)/sizeof(L3_Branch1[0]));

    // Connect Kit Kat to the start of Branch 1
    metroGraph.addEdge("Kit Kat", "El Sudan");

    // Line 3 - Branch 2 (Kit Kat -> Cairo University)
    // Add new vertices for the recently opened phase
    metroGraph.addVertex("El Tawfikeya");
    metroGraph.addVertex("Wadi El Nile");
    metroGraph.addVertex("Gameat El Dowal");

    // Connect Kit Kat to the start of Branch 2 (El Tawfikeya)
    metroGraph.addEdge("Kit Kat", "El Tawfikeya");
    metroGraph.addEdge("El Tawfikeya", "Wadi El Nile");
    metroGraph.addEdge("Wadi El Nile", "Gameat El Dowal");
    // Connects to Line 2 (Cairo University interchange)
    metroGraph.addEdge("Gameat El Dowal", "Cairo University");

    cout << "Metro Network Setup Complete: " << metroGraph.getVertexCount() << " Stations Loaded.\n";

    // =========================================================
    // User Menu
    // =========================================================

    string item, item2;
    int num;

    do {
        displayOrders();
        cout << "Enter option: ";
        if (!(cin >> num)) {
            cin.clear();
            cin.ignore(10000, '\n');
            num = -1;
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        cin.ignore();

        switch (num) {
            case 1:  // Add Station
                cout << "\nEnter Station Name: ";
                getline(cin, item);
                metroGraph.addVertex(item);
                cout << "Station '" << item << "' added (or already exists).\n";
                break;
            case 2:  // Add Line Segment
                cout << "\nFrom Station: ";
                getline(cin, item);
                cout << "To Station: ";
                getline(cin, item2);
                metroGraph.addEdge(item, item2);
                cout << "Edge added between '" << item << "' and '" << item2 << "'.\n";
                break;
            case 3:  // Find Shortest Path (By Stations Count)
                cout << "\nEnter Starting Station: ";
                getline(cin, item);
                cout << "Enter Destination Station: ";
                getline(cin, item2);
                metroGraph.findShortestPath(item, item2);
                break;
            case 4:  // Display All Connections
                cout << "\n--- Metro Connections ---\n";
                metroGraph.displayGraph();
                cout << "---------------------------\n";
                break;
            case 0:
                cout << "Exiting Metro Analyzer. Goodbye!\n";
                break;
            default:
                cout << "Invalid option! Please try again.\n";
        }
    }while (num != 0);

    return 0;
}
