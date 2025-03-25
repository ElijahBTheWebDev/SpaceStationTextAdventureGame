#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>  // For usleep function
#include <cstdlib>  // For rand() function
#include <ctime>    // For time()
#include <cstdio>   // For sprintf function
#include <cctype>   // For isalpha function

using namespace std;

// Add forward declaration at the top
class Game;  // Forward declaration

class Item {
    public:
        string name;
        string description;
        
        Item(string n, string d) {
            name = n;
            description = d;
        }
        
        void examine(Game* game);  // Just declare the function here
};

class Room {
    public:
        string name;
        string description;
        vector<Item> items;
        
        Room(string n, string d) {
            name = n;
            description = d;
        }
};

class Game {
    public:
        static const int MAX_INVENTORY = 7;  // Increase from 6 to 7 items
        static const int TEXT_WIDTH = 60;  // Narrower width for better readability
        static const int INDENT_SIZE = 4;  // Spaces for paragraph indentation
        vector<Room> rooms;
        vector<Item> inventory;
        int currentRoom;
        bool airlockDoorOpen = false;
        bool hasLight = false;        // Track if player has working light
        int actionCounter = 0;        // Count actions after entering maintenance
        bool inMaintenance = false;   // Track if player has entered maintenance
        bool obsdeckDoorUnlocked = false;
        const string DOOR_CODE = "9572";
        const string CONTROL_CODE = "1701";  // New code for Control Room
        bool computerSystemFixed = false;
        bool navigationSystemFixed = false;
        bool lifeSupportFixed = false;
        vector<bool> roomFirstVisit;  // Track if each room has been visited
        vector<bool> roomSearched;  // Track if each room has been searched
        bool suitDamaged = false;
        int commandsUntilDeath = 15;  // Number of commands before oxygen depletion
        bool suitRepaired = false;
        int messHallCounter = 0;  // Counter for mess hall commands
        bool messHallCounterStarted = false;  // Track if counter has started
        bool hasGlowStickLight = false;  // Track glow stick light separately
        bool blowTorchFueled = false;  // Track if blow torch has fuel
        bool controlRoomDoorOpen = false;  // Track if control room door has been opened
        bool feelAroundUsed = false;  // Track if feel around was already used in current room
        
        Game() {
            srand(time(NULL));  // Seed random number generator
            initializeGame();
        }
        
        void initializeGame() {
            clearScreen();
            
            // Create rooms and add items first
            rooms.push_back(Room("Airlock", "A pressurized chamber with heavy metal doors. A tool box sits in the corner."));
            rooms.push_back(Room("Maintenance Corridor", "A long, dark narrow hallway."));
            rooms.push_back(Room("Observation Deck", "Large windows show the vast expanse of space."));
            rooms.push_back(Room("Mess Hall", "Tables and storage cabinets line the walls. Food trays are scattered about."));
            rooms.push_back(Room("Control Room", "Banks of computers line the walls. Most screens are dark."));
            
            // Add items to rooms
            rooms[0].items.push_back(Item("Crowbar", "A sturdy metal crowbar from the tool box. Could be useful for prying things open."));
            rooms[0].items.push_back(Item("Duct Tape", "A roll of industrial strength duct tape. Universal repair tool."));
            rooms[0].items.push_back(Item("Pressure Gauge", "A digital gauge showing dangerous fluctuations in the station's air pressure."));
            
            rooms[1].items.push_back(Item("Glow Stick", "A bright emergency glow stick. Provides reliable light in dark areas."));
            rooms[1].items.push_back(Item("Wire Cutters", "Heavy-duty cutting tool. Perfect for electrical repairs and wire management."));
            rooms[1].items.push_back(Item("Repair Manual", "A worn technical manual detailing station maintenance procedures."));
            rooms[1].items.push_back(Item("Sticky Note", "A crumpled yellow sticky note with hastily scrawled numbers. It reads: 'Observation Deck Security Code: " + DOOR_CODE + "'"));
            
            rooms[2].items.push_back(Item("Blow Torch", "A portable welding torch. Needs fuel to operate."));
            rooms[2].items.push_back(Item("Star Chart", "A holographic display showing local star systems. Might help with navigation."));
            rooms[2].items.push_back(Item("Telescope Lens", "A cracked lens from the observation equipment. Still usable as a focusing tool."));
            rooms[2].items.push_back(Item("Radio", "A short-range communication device. No response on any emergency channels."));
            rooms[2].items.push_back(Item("Circuit Board", "A replacement computer circuit board. Looks compatible with the main system."));
            
            rooms[3].items.push_back(Item("Water Container", "An emergency water storage unit. Essential for survival in space."));
            rooms[3].items.push_back(Item("First Aid Kit", "A well-stocked medical kit. Contains various supplies for emergencies."));
            rooms[3].items.push_back(Item("Butane Canister", "A canister of butane fuel. Compatible with standard welding equipment."));
            rooms[3].items.push_back(Item("9V Batteries", "A fresh pack of 9V batteries. Standard power source for emergency equipment."));
            rooms[3].items.push_back(Item("Energy Bar", "A high-calorie emergency ration bar. Still within its expiration date."));
            
            rooms[4].items.push_back(Item("ASCII Table", "A data pad containing station protocols and ASCII reference data."));
            
            // First, add a clue to the Control Room items during initialization
            rooms[4].items.push_back(Item("Sticky Note", "A crumpled sticky note with hexadecimal numbers scrawled on it: '4F 56 45 52 52 49 44 45'"));
            
            // Start with headlight in inventory
            inventory.push_back(Item("Headlight", "A battery-powered LED headlight. Essential for dark areas."));
            
            currentRoom = 0;
            
            // Update in initializeGame()
            cout << "\n=== EMERGENCY ALERT ===\n\n";

            wrapText("Multiple critical systems are down aboard the space station:", false);
            cout << "\n";
            wrapText("- Life Support System: Critical Failure", false);
            wrapText("- Navigation System: Offline", false);
            wrapText("- Computer Systems: Malfunctioning", false);
            cout << "\n\n";

            wrapText("Mission Objectives:", false);
            cout << "\n";
            wrapText("1. Make your way through the space station to reach the Control Room", false);
            cout << "\n";
            wrapText("2. Collect necessary tools and equipment", false);
            cout << "\n";
            wrapText("3. Restore all critical systems (Navigation, Life Support, and Computer Systems)", false);
            cout << "\n\n";

            wrapText("Press Enter to begin emergency protocols...", false);
            cin.get();

            clearScreen();
            wrapText("Current Location: Airlock", true, "info");
            cout << "\n";

            roomFirstVisit = vector<bool>(rooms.size(), true);  // Initialize all rooms as unvisited
            roomSearched = vector<bool>(rooms.size(), false);  // Initialize all rooms as unsearched
        }
        
        void parseCommand(string input) {
            // Convert input to lowercase for easier comparison
            string lowerInput = input;
            for (char& c : lowerInput) {
                c = tolower(c);
            }

            // Show oxygen warning first and keep it visible
            if (suitDamaged && !suitRepaired) {
                commandsUntilDeath--;
                clearScreen();
                if (commandsUntilDeath <= 0) {
                    wrapText("Your suit's oxygen supply is depleted. The room begins to spin as you lose consciousness...", false);
                    cout << "\n\nGame Over\n";
                    exit(0);
                }
                else {
                    wrapText("WARNING: Suit oxygen leak active. Commands remaining: " + to_string(commandsUntilDeath), false, "alert");
                    if (commandsUntilDeath <= 3) {
                        wrapText("CRITICAL: Seal the leak immediately!", false, "alert");
                    }
                    cout << "\n";
                }
            }

            // Update mess hall light failure - trigger on first command in mess hall
            if (currentRoom == 3) {  // In Mess Hall
                if (!messHallCounterStarted) {
                    messHallCounterStarted = true;
                    hasLight = false;  // Turn off headlight
                    clearScreen();
                    wrapText("Your headlight suddenly flickers and dies. The batteries are completely drained!", false, "alert");
                    cout << "\n";
                    wrapText("The mess hall is plunged into darkness...", false);
                    cout << "\n";
                    wrapText("Maybe you could try searching around in the dark...", false, "info");
                    cout << "\n\n";
                    return;
                }
                
                // Move battery check here after lowerInput is defined
                if (!hasLight && (lowerInput == "search" || lowerInput == "s")) {
                    messHallCounter++;
                    if (messHallCounter == 3) {
                        clearScreen();
                        wrapText("After fumbling in the darkness, your hand brushes against something familiar...", false);
                        cout << "\n";
                        wrapText("You found: 9V Batteries! You replace the batteries in your headlight, and turn it on!", false, "info");
                        inventory.push_back(Item("9V Batteries", "A fresh pack of 9V batteries. Standard power source for emergency equipment."));
                        hasLight = true;  // Restore light
                        cout << "\n";
                        return;
                    }
                }
            }

            // Check if we need light for the current command
            bool needsLight = (lowerInput == "search" || lowerInput == "s" || 
                              lowerInput == "examine" || lowerInput == "e" ||
                              lowerInput == "move" || lowerInput == "m" || 
                              lowerInput.find("move") != string::npos);

            if (needsLight && !hasLightSource()) {
                clearScreen();  // Add this line
                wrapText("It's too dark to do that. You need a light source.", false, "alert");
                cout << "\n";
                return;
            }

            // Movement commands
            if (input == "M" || input == "Move" || 
                lowerInput == "go to next room" || 
                lowerInput == "open door" ||
                lowerInput == "go forward" ||
                lowerInput == "continue forward" ||
                lowerInput == "proceed" ||
                lowerInput == "go ahead" ||
                lowerInput == "next room" ||
                lowerInput.find("move to") != string::npos || 
                lowerInput.find("go to") != string::npos) {
                moveToNextRoom();
                return;
            }

            // Search commands
            if (lowerInput == "look around" ||
                lowerInput == "check room" ||
                lowerInput == "search room" ||
                lowerInput == "examine room" ||
                lowerInput == "scan room" ||
                lowerInput == "inspect" ||
                lowerInput == "investigate" ||
                lowerInput == "s" ||        // Add shortcut
                lowerInput == "search") {   // Add basic search command
                search();
                return;
            }

            // Take/Grab commands
            if (lowerInput == "g" || lowerInput == "grab" || lowerInput == "take" ||
                lowerInput.find("grab ") != string::npos ||
                lowerInput.find("take ") != string::npos ||
                lowerInput.find("pick up") != string::npos ||
                lowerInput.find("take the") != string::npos ||
                lowerInput.find("grab the") != string::npos ||
                lowerInput.find("get the") != string::npos) {
                
                // Handle item name if provided
                if (lowerInput.find(" the ") != string::npos) {
                    size_t pos = lowerInput.find(" the ");
                    if (pos != string::npos) {
                        string argument = lowerInput.substr(pos + 5);
                        takeItem(argument);
                        return;
                    }
                } else if (lowerInput.find(" ") != string::npos) {
                    string argument = lowerInput.substr(lowerInput.find(" ") + 1);
                    takeItem(argument);
                    return;
                } else {
                    takeItem("");  // Show inventory list if no item specified
                    return;
                }
            }

            // Examine commands
            if (lowerInput == "e" || 
                lowerInput == "examine" ||
                lowerInput.find("look at") != string::npos ||
                lowerInput.find("check the") != string::npos ||
                lowerInput.find("examine the") != string::npos ||
                lowerInput.find("inspect the") != string::npos) {
                
                // Handle item name if provided
                if (lowerInput.find(" the ") != string::npos) {
                    size_t pos = lowerInput.find(" the ");
                    if (pos != string::npos) {
                        string argument = lowerInput.substr(pos + 5);
                        examineItem(argument);
                        return;
                    }
                } else if (lowerInput.find(" ") != string::npos) {
                    string argument = lowerInput.substr(lowerInput.find(" ") + 1);
                    examineItem(argument);
                    return;
                } else {
                    examineItem("");  // Show list of items to examine if no item specified
                    return;
                }
            }

            // Map commands
            if (input == "m" || input == "map" || input == "Map" || 
                lowerInput == "show map" || 
                lowerInput == "display map" ||
                lowerInput == "view map" ||
                lowerInput == "check map" ||
                lowerInput == "where am i") {
                showMap();
                return;
            }

            // Help commands
            if (lowerInput == "what can i do" ||
                lowerInput == "show commands" ||
                lowerInput == "show help" ||
                lowerInput == "commands" ||
                lowerInput == "options" ||
                lowerInput == "help" ||     // Add basic help command
                lowerInput == "h") {        // Add shortcut
                showHelp();
                return;
            }

            // Handle inventory and info variations
            if (input == "I" || input == "inv" || input == "inventory") {
                listInventory();
                return;
            }
            if (lowerInput == "room info" || lowerInput == "info" || lowerInput == "i") {
                showRoomInfo();
                return;
            }

            // Exploration/Interaction commands
            if (lowerInput == "feel around" || 
                lowerInput == "feel" || 
                lowerInput == "touch around" || 
                lowerInput == "fumble around" ||
                lowerInput == "grope around" ||
                lowerInput == "reach around" ||
                lowerInput == "search with hands" ||
                lowerInput == "search by touch" ||
                lowerInput == "search in dark" ||
                lowerInput == "search blindly" ||
                lowerInput == "feel in dark" ||
                lowerInput == "feel your way" ||
                lowerInput == "feel way around" ||
                lowerInput == "use hands to search" ||
                lowerInput == "search by feeling") {
                feelAround();
                return;
            }

            // More descriptive error messages
            if (lowerInput == "go") {
                cout << "To move to the next room, try 'move' or 'move to next room'.\n";
            }
            else if (lowerInput == "get" || lowerInput == "grab" || lowerInput == "pickup") {
                cout << "To pick up items, use the 'take [item name]' command.\n";
            }
            else if (lowerInput == "look" || lowerInput == "check") {
                cout << "To look around, use the 'search' command.\n";
            }
            else if (lowerInput == "inventory" || lowerInput == "inv") {
                cout << "To check your inventory, use 'view inventory'.\n";
            }
            else {
                cout << "Unknown command '" << lowerInput << "'. Type 'help' for available commands.\n";
            }

            // In parseCommand() function, add butane torch command check
            if (lowerInput == "use butane torch" || lowerInput == "use blow torch with butane") {
                bool hasBlowtorch = false;
                bool hasButane = false;
                
                // Check for both items
                for (const Item& item : inventory) {
                    if (item.name == "Blow Torch") hasBlowtorch = true;
                    if (item.name == "Butane Canister") hasButane = true;
                }
                
                if (hasBlowtorch && hasButane) {
                    clearScreen();
                    wrapText("You attach the butane canister to the blow torch and begin melting the lock on the door to the control room.", false);
                    cout << "\n\n";
                    wrapText("The lock mechanism glows red hot and finally gives way.", false);
                    cout << "\n";
                    blowTorchFueled = true;
                    
                    // Remove butane canister from inventory
                    for (int i = 0; i < inventory.size(); i++) {
                        if (inventory[i].name == "Butane Canister") {
                            inventory.erase(inventory.begin() + i);
                            break;
                        }
                    }
                    return;
                } else if (!hasBlowtorch) {
                    wrapText("You need a blow torch first.", false);
                    cout << "\n";
                    return;
                } else if (!hasButane) {
                    wrapText("You need a butane canister for the torch.", false);
                    cout << "\n";
                    return;
                }
            }

            // In parseCommand() function, add use command check
            if (lowerInput == "u" || 
                lowerInput == "use" || 
                lowerInput.find("use the") != string::npos ||
                lowerInput.find("use ") != string::npos) {
                if (lowerInput.find(" the ") != string::npos) {
                    size_t pos = lowerInput.find(" the ");
                    if (pos != string::npos) {
                        string argument = lowerInput.substr(pos + 5);
                        useItem(argument);
                        return;
                    }
                } else if (lowerInput.find("use ") != string::npos) {
                    string argument = lowerInput.substr(4);
                    useItem(argument);
                    return;
                } else {
                    useItem("");  // Show inventory list if no item specified
                    return;
                }
            }

            // In parseCommand() function, add computer terminal command check
            if (lowerInput == "use computer" || 
                lowerInput == "use terminal" ||
                lowerInput == "use computer terminal" ||
                lowerInput == "use main computer" ||
                lowerInput == "access computer" ||
                lowerInput == "access terminal" ||
                lowerInput == "use main computer system" ||
                lowerInput == "use computer system" ||
                lowerInput == "access main computer") {
                
                if (currentRoom == 4) {  // If in Control Room
                    examineSystem("computer");
                    return;
                } else {
                    wrapText("There is no computer terminal here.", false);
                    cout << "\n";
                    return;
                }
            }

            // In parseCommand() function, add drop commands
            if (lowerInput == "d" || 
                lowerInput == "drop" ||
                lowerInput.find("drop ") != string::npos ||
                lowerInput.find("drop the") != string::npos) {
                
                // Handle item name if provided
                if (lowerInput.find(" the ") != string::npos) {
                    size_t pos = lowerInput.find(" the ");
                    if (pos != string::npos) {
                        string argument = lowerInput.substr(pos + 5);
                        dropItem(argument);
                        return;
                    }
                } else if (lowerInput.find(" ") != string::npos) {
                    string argument = lowerInput.substr(lowerInput.find(" ") + 1);
                    dropItem(argument);
                    return;
                } else {
                    dropItem("");  // Show inventory list if no item specified
                    return;
                }
            }
        }

        void wrapText(string text, bool indent = false, string style = "normal") {
            istringstream words(text);
            string word;
            string line;
            string indentation = indent ? string(INDENT_SIZE, ' ') : "";
            bool firstLine = true;
            
            // Apply style formatting without indentation
            if (style == "alert") {
                cout << "! ";  // Alert prefix
            } else if (style == "info") {
                cout << "* ";  // Info prefix
            }
            
            while (words >> word) {
                // Handle first line indentation
                if (firstLine) {
                    line = style == "normal" ? indentation : "";  // Only indent normal text
                    firstLine = false;
                }
                
                // If this word would make line too long
                if (line.length() + word.length() + 1 > TEXT_WIDTH) {
                    cout << line << "\n";
                    line = style == "normal" ? indentation : "";  // Only indent normal text
                    line += word;
                } else {
                    if (!line.empty() && line != indentation) {
                        line += " ";
                    }
                    line += word;
                }
            }
            
            // Print last line if anything remains
            if (!line.empty()) {
                cout << line << "\n";
            }
        }

        void search() {
            clearScreen();
            cout << "\nYou are in the " << rooms[currentRoom].name << "\n\n";

            if (!hasLight && (currentRoom == 0 || currentRoom == 1)) {
                if (currentRoom == 0) {
                    wrapText("Darkness fills the airlock.", false);
                    cout << "\n";
                    wrapText("The emergency lights have failed, leaving only the faint glow of distant stars through the small window.", false);
                    cout << "\n";
                } else {
                    wrapText("The maintenance corridor is completely dark.", false);
                    cout << "\n";
                    wrapText("You can hear the creaking of metal and the soft whoosh of air through the ventilation system.", false);
                    cout << "\n";
                }

                // Add 50% chance to find random item in dark
                if (!rooms[currentRoom].items.empty()) {
                    if (rand() % 2 == 0) {  // 50% chance
                        int randomIndex = rand() % rooms[currentRoom].items.size();
                        Item foundItem = rooms[currentRoom].items[randomIndex];
                        
                        if (inventory.size() >= MAX_INVENTORY) {
                            wrapText("You stumble upon something in the darkness, but your inventory is full!", false);
                            cout << "\n";
                        } else {
                            wrapText("Despite the darkness, your hand brushes against something...", false);
                            cout << "\n";
                            inventory.push_back(foundItem);
                            rooms[currentRoom].items.erase(rooms[currentRoom].items.begin() + randomIndex);
                            wrapText("You found: " + foundItem.name, false);
                            cout << "\n";
                        }
                    }
                }
                
                if (actionCounter >= 15) {
                    wrapText("Somewhere in the darkness ahead, you notice a faint green glow.", true, "info");
                }
                cout << "\n";
                return;
            }

            roomSearched[currentRoom] = true;  // Mark room as searched

            // Only show items when searching
            if (!rooms[currentRoom].items.empty()) {
                wrapText("After searching the room, you find:", false);
                cout << "\n";
                for (int i = 0; i < rooms[currentRoom].items.size(); i++) {
                    cout << string(INDENT_SIZE, ' ') << i + 1 << ". " << rooms[currentRoom].items[i].name << "\n";
                }
                cout << "\n";
            } else {
                wrapText("You search the room but find no useful items.", true, "info");
                cout << "\n";
            }
            
            // Add special terminal notifications for each room
            if (currentRoom == 1) {  // Maintenance Corridor
                wrapText("You also notice:", false);
                cout << "\n";
                wrapText("- An Observation Deck Security Terminal", true);
                wrapText("- A Life Support System Access Terminal", true);
                cout << "\n";
            } 
            else if (currentRoom == 2) {  // Observation Deck
                wrapText("You also notice:", false);
                cout << "\n";
                wrapText("- A Navigation System Terminal", true);
                wrapText("- A Mess Hall Security Terminal", true);
                cout << "\n";
            }
            else if (currentRoom == 4) {  // Control Room
                wrapText("You also notice:", false);
                cout << "\n";
                wrapText("- A Main Computer System Terminal", true);
                cout << "\n";
            }
            
            checkAndUpdateLight();
        }

        void checkAndUpdateLight() {
            if (inMaintenance) {
                actionCounter++;
                if (actionCounter == 15) {
                    cout << "\nYour headlight flickers and dies. The batteries are dead!\n";
                    hasLight = false;
                }
            }
        }

        void moveToNextRoom() {
            clearScreen();
            
            if (currentRoom == 0) {  // In Airlock
                if (!airlockDoorOpen) {
                    wrapText("The airlock door is sealed tight. The emergency override appears to be malfunctioning.", false);
                    cout << "\n";
                    wrapText("You'll need to find a way to force it open.", false);
                    cout << "\n";
                    return;
                }
                // If door is open, move directly to maintenance
                currentRoom = 1;
                clearScreen();
                wrapText("Moving to the Maintenance Corridor...", true);
                cout << "\n";
                if (roomFirstVisit[currentRoom]) {
                    wrapText("The maintenance corridor stretches before you, a claustrophobic tunnel. Through your helmet's visor, you can see damaged electrical systems sparking in the darkness.", true);
                    roomFirstVisit[currentRoom] = false;
                }
                feelAroundUsed = false;  // Add this line when room changes
                return;
            }

            // Rest of the function for other rooms...
            if (currentRoom == 1) {
                cout << "Which direction would you like to move?\n\n";
                cout << "1. Back to Airlock\n";
                cout << "2. Forward to Observation Deck\n";
                cout << "\nEnter choice (or 0 to cancel): ";
                
                int choice;
                if (!getNumericInput(choice, 2)) {
                    cout << "Invalid input. Please enter 0, 1, or 2.\n";
                    return;
                }
                
                if (choice == 0) return;
                
                if (choice == 1) {
                    currentRoom = 0;
                    cout << "\nYou return to the Airlock.\n";
                    return;
                }
                
                if (choice == 2) {  // Moving to Observation Deck
                    // Add suit damage check here
                    if (!suitDamaged && !suitRepaired && currentRoom == 1) {  // In corridor and suit not damaged yet
                        suitDamaged = true;
                        clearScreen();
                        wrapText("\nAs you reach for the observation deck door controls, your suit catches on a jagged piece of torn metal!", false, "alert");
                        cout << "\n";
                        wrapText("WARNING: Suit integrity compromised. Oxygen leak detected. Estimated 5 minutes of breathable air remaining.", false, "alert");
                        cout << "\n";
                        wrapText("You need to seal the tear quickly!", false, "alert");
                        cout << "\n";
                        return;
                    }

                    // Add security terminal check
                    if (!obsdeckDoorUnlocked) {
                        clearScreen();
                        terminalEffect("\n=== OBSERVATION DECK SECURITY TERMINAL ===\n");
                        terminalEffect("Accessing security systems...");
                        terminalEffect("Initiating authentication protocol...\n");
                        
                        cout << "\nEnter security code (or 0 to cancel): ";
                        string input;
                        getline(cin, input);
                        
                        if (input == "0") {
                            terminalEffect("Terminal session terminated.");
                            return;
                        }
                        
                        terminalEffect("Validating code...");
                        usleep(1000000);  // 1 second pause
                        
                        if (input == DOOR_CODE) {
                            terminalEffect("ACCESS GRANTED", 100000);
                            terminalEffect("Disengaging security locks...");
                            terminalEffect("Opening observation deck doors...\n");
                            obsdeckDoorUnlocked = true;
                            currentRoom = 2;
                            cout << "\n";
                            wrapText("Current Location: Observation Deck", true, "info");
                            cout << "\n";
                            
                            // Show room info on first entry
                            if (roomFirstVisit[currentRoom]) {
                                showRoomInfo();
                                roomFirstVisit[currentRoom] = false;
                            }
                        } else {
                            terminalEffect("ACCESS DENIED", 100000);
                            terminalEffect("Invalid security code. Terminal locked for 5 seconds.");
                            for (int i = 5; i > 0; i--) {
                                cout << i << "..." << flush;
                                usleep(1000000);
                            }
                            cout << "\n";
                        }
                        return;
                    }
                    
                    // If door is unlocked, allow movement
                    if (obsdeckDoorUnlocked) {
                        currentRoom = 2;
                        cout << "\nYou enter the Observation Deck.\n";
                    }
                    return;
                }
                return;
            }

            // In Observation Deck
            if (currentRoom == 2) {
                cout << "Which direction would you like to move?\n\n";
                cout << "1. Back to Maintenance Corridor\n";
                cout << "2. Forward to Mess Hall\n";
                cout << "\nEnter choice (or 0 to cancel): ";
                
                int choice;
                if (!getNumericInput(choice, 2)) {
                    cout << "Invalid input. Please enter 0, 1, or 2.\n";
                    return;
                }
                
                if (choice == 0) return;
                
                if (choice == 1) {
                    currentRoom = 1;
                    cout << "\nYou return to the Maintenance Corridor.\n";
                    return;
                }
                
                if (choice == 2) {  // Moving to Mess Hall
                    clearScreen();
                    terminalEffect("\n=== MESS HALL SECURITY TERMINAL ===\n");
                    terminalEffect("Accessing security systems...");
                    terminalEffect("Initiating authentication protocol...\n");
                    
                    cout << "\nEnter security code (or 0 to cancel): ";
                    string input;
                    getline(cin, input);
                    
                    if (input == "0") {
                        terminalEffect("Terminal session terminated.");
                        return;
                    }
                    
                    terminalEffect("Validating code...");
                    usleep(1000000);  // 1 second pause
                    
                    if (input == DOOR_CODE) {
                        terminalEffect("ACCESS GRANTED", 100000);
                        terminalEffect("Disengaging security locks...");
                        terminalEffect("Opening mess hall doors...\n");
                        currentRoom = 3;
                        cout << "\n";
                        wrapText("Current Location: Mess Hall", true, "info");
                        cout << "\n";
                        
                        // Show room info on first entry
                        if (roomFirstVisit[currentRoom]) {
                            showRoomInfo();
                            roomFirstVisit[currentRoom] = false;
                        }
                    } else {
                        terminalEffect("ACCESS DENIED", 100000);
                        terminalEffect("Invalid security code. Terminal locked for 5 seconds.");
                        for (int i = 5; i > 0; i--) {
                            cout << i << "..." << flush;
                            usleep(1000000);
                        }
                        cout << "\n";
                    }
                    return;
                }
            }

            // In Mess Hall
            if (currentRoom == 3) {
                cout << "Which direction would you like to move?\n\n";
                cout << "1. Back to Observation Deck\n";
                cout << "2. Forward to Control Room\n";
                cout << "\nEnter choice (or 0 to cancel): ";
                
                int choice;
                if (!getNumericInput(choice, 2)) {
                    cout << "Invalid input. Please enter 0, 1, or 2.\n";
                    return;
                }
                
                if (choice == 0) return;
                
                if (choice == 1) {
                    currentRoom = 2;
                    cout << "\nYou return to the Observation Deck.\n";
                    return;
                }
                
                if (choice == 2) {  // Moving to Control Room
                    if (!controlRoomDoorOpen) {
                        clearScreen();  // Add this line
                        wrapText("The control room door is sealed shut. You'll need to find a way to cut through the emergency locks.", false);
                        cout << "\n";
                        return;
                    }
                    
                    // If door was cut open with torch, skip terminal and move directly to control room
                    currentRoom = 4;
                    cout << "\n";
                    wrapText("Current Location: Control Room", true, "info");
                    cout << "\n";
                    
                    // Show room info on first entry
                    if (roomFirstVisit[currentRoom]) {
                        showRoomInfo();
                        roomFirstVisit[currentRoom] = false;
                    }
                    return;
                }
            }

            // Show available rooms
            cout << "Available rooms to move to:\n\n";
            
            int optionNumber = 1;  // Always start at 1
            
            if (currentRoom > 0) {
                cout << optionNumber++ << ". Go back to " << rooms[currentRoom-1].name << "\n";
            }
            if (currentRoom < rooms.size() - 1) {
                cout << optionNumber << ". Continue to " << rooms[currentRoom+1].name << "\n";
            }
            
            cout << "\nEnter number (or 0 to cancel): ";
            int choice;
            if (!getNumericInput(choice, rooms.size() - 1)) {
                cout << "Invalid input. Please enter a number between 0 and " << rooms.size() - 1 << ".\n";
                return;
            }

            if (choice == 1) {
                if (currentRoom > 0) {
                    currentRoom--;
                    clearScreen();
                    wrapText("Moving back to the " + rooms[currentRoom].name + "...", true);
                    cout << "\n";
                    if (roomFirstVisit[currentRoom]) {
                        switch(currentRoom) {
                            case 0:
                                wrapText("The airlock chamber hisses softly as pressure equalizes. Emergency backup lights cast long shadows across the curved metal walls. The faint glow of distant stars filters through the thick observation window, barely illuminating the essential equipment stored here.", true);
                                break;
                            case 1:
                                wrapText("The maintenance corridor stretches before you, a claustrophobic tunnel of exposed infrastructure. Through your helmet's visor, you can see damaged electrical systems sparking in the darkness.", true);
                                break;
                            case 2:
                                wrapText("The observation deck opens up into a vast panorama of stars. The reinforced windows span from floor to ceiling, offering a breathtaking view of the infinite void. Navigation equipment blinks silently, their displays casting a soft blue glow across the abandoned workstations.", true);
                                break;
                            case 3:
                                wrapText("The mess hall stands frozen in time - half-eaten meals still sitting on tables, chairs askew as if hastily abandoned. The gentle hum of food preservation units provides an eerie backdrop to the scene of interrupted daily life.", true);
                                break;
                            case 4:
                                wrapText("Banks of computers line the walls of the control room, their screens flickering with intermittent power. Status displays flash urgent warnings in red and amber, casting an unsettling glow across the primary command console. This is the brain of the station, and it's clearly unwell.", true);
                                break;
                        }
                        cout << "\n";
                        roomFirstVisit[currentRoom] = false;  // Mark room as visited
            } else {
                        wrapText(rooms[currentRoom].description, true);  // Show basic description for subsequent visits
                    }
                    cout << "\n";
                } else {
                    currentRoom++;
                    clearScreen();
                    switch(currentRoom) {
                        case 1:
                            wrapText("You pry your way through the airlock door into the maintenance corridor.", true);
                            break;
                        case 2:
                            wrapText("You carefully navigate through the dark corridor to the observation deck.", true);
                            break;
                        case 3:
                            wrapText("You enter the mess hall.", true);
                            break;
                        case 4:
                            wrapText("You make your way to the control room.", true);
                            break;
                    }
                    cout << "\n";
                    if (currentRoom == 1) {
                        inMaintenance = true;
                    }
                    if (roomFirstVisit[currentRoom]) {
                        switch(currentRoom) {
                            case 0:
                                wrapText("The airlock chamber hisses softly as pressure equalizes. Emergency backup lights cast long shadows across the curved metal walls. The faint glow of distant stars filters through the thick observation window, barely illuminating the essential equipment stored here.", true);
                                break;
                            case 1:
                                wrapText("The maintenance corridor stretches before you, a claustrophobic tunnel of exposed infrastructure. Through your helmet's visor, you can see damaged electrical systems sparking in the darkness.", true);
                                break;
                            case 2:
                                wrapText("The observation deck opens up into a vast panorama of stars. The reinforced windows span from floor to ceiling, offering a breathtaking view of the infinite void. Navigation equipment blinks silently, their displays casting a soft blue glow across the abandoned workstations.", true);
                                break;
                            case 3:
                                wrapText("The mess hall stands frozen in time - half-eaten meals still sitting on tables, chairs askew as if hastily abandoned. The gentle hum of food preservation units provides an eerie backdrop to the scene of interrupted daily life.", true);
                                break;
                            case 4:
                                wrapText("Banks of computers line the walls of the control room, their screens flickering with intermittent power. Status displays flash urgent warnings in red and amber, casting an unsettling glow across the primary command console. This is the brain of the station, and it's clearly unwell.", true);
                                break;
                        }
                        cout << "\n";
                        roomFirstVisit[currentRoom] = false;  // Mark room as visited
                    }
                    wrapText(rooms[currentRoom].description, true);
                    cout << "\n";
                }
            }
            else if (choice == 2 && currentRoom > 0 && currentRoom < rooms.size() - 1) {
                currentRoom++;
                clearScreen();
                switch(currentRoom) {
                    case 1:
                        wrapText("You pry your way through the airlock door into the maintenance corridor.", true);
                        break;
                    case 2:
                        wrapText("You carefully navigate through the dark corridor to the observation deck.", true);
                        break;
                    case 3:
                        wrapText("You enter the mess hall.", true);
                        break;
                    case 4:
                        wrapText("You make your way to the control room.", true);
                        break;
                }
                cout << "\n";
                if (currentRoom == 1) {
                    inMaintenance = true;
                }
                if (roomFirstVisit[currentRoom]) {
                    switch(currentRoom) {
                        case 0:
                            wrapText("The airlock chamber hisses softly as pressure equalizes. Emergency backup lights cast long shadows across the curved metal walls. The faint glow of distant stars filters through the thick observation window, barely illuminating the essential equipment stored here.", true);
                            break;
                        case 1:
                            wrapText("The maintenance corridor stretches before you, a claustrophobic tunnel of exposed infrastructure. Through your helmet's visor, you can see damaged electrical systems sparking in the darkness.", true);
                            break;
                        case 2:
                            wrapText("The observation deck opens up into a vast panorama of stars. The reinforced windows span from floor to ceiling, offering a breathtaking view of the infinite void. Navigation equipment blinks silently, their displays casting a soft blue glow across the abandoned workstations.", true);
                            break;
                        case 3:
                            wrapText("The mess hall stands frozen in time - half-eaten meals still sitting on tables, chairs askew as if hastily abandoned. The gentle hum of food preservation units provides an eerie backdrop to the scene of interrupted daily life.", true);
                            break;
                        case 4:
                            wrapText("Banks of computers line the walls of the control room, their screens flickering with intermittent power. Status displays flash urgent warnings in red and amber, casting an unsettling glow across the primary command console. This is the brain of the station, and it's clearly unwell.", true);
                            break;
                    }
                    cout << "\n";
                    roomFirstVisit[currentRoom] = false;  // Mark room as visited
                }
                wrapText(rooms[currentRoom].description, true);
                cout << "\n";
            }
            
            checkAndUpdateLight();
        }

        void listInventory() {
            clearScreen();
            if (inventory.empty()) {
                wrapText("Your inventory is empty.", true, "info");
                return;
            }
            wrapText("Inventory (" + to_string(inventory.size()) + "/" + to_string(MAX_INVENTORY) + " items):", false);
            cout << "\n";
            for (int i = 0; i < inventory.size(); i++) {
                cout << string(INDENT_SIZE, ' ') << i + 1 << ". " << inventory[i].name << "\n";
            }
        }

        void takeItem(string itemName) {
            clearScreen();
            
            // Check for light in dark rooms first
            if (!hasLight && (currentRoom == 0 || currentRoom == 1)) {
                wrapText("The darkness makes it impossible to find anything. You'll need a light source first.", false, "alert");
                cout << "\n";
                return;
            }

            // Remove the initial search check since we want to allow taking items from previously searched rooms
            
            // If no item specified, show numbered list
            if (itemName.empty()) {
                if (rooms[currentRoom].items.empty()) {
                    cout << "There are no items to take here.\n";
                    return;
                }

                cout << "What do you want to grab?\n\n";
                for (int i = 0; i < rooms[currentRoom].items.size(); i++) {
                    cout << i + 1 << ". " << rooms[currentRoom].items[i].name << "\n";
                }

                cout << "\nEnter number (or 0 to cancel): ";
                int choice;
                if (!getNumericInput(choice, rooms[currentRoom].items.size())) {
                    cout << "Invalid input. Please enter a number between 0 and " << rooms[currentRoom].items.size() << ".\n";
                    return;
                }

                clearScreen();
                if (choice > 0 && choice <= rooms[currentRoom].items.size()) {
                    if (inventory.size() >= MAX_INVENTORY) {
                        cout << "Your inventory is full! Drop something first.\n";
                        return;
                    }

                    Item selectedItem = rooms[currentRoom].items[choice - 1];
                    if (selectedItem.name == "Pressure Gauge") {
                        cout << "The pressure gauge is securely mounted to the wall.\n";
                        return;
                    }

                    inventory.push_back(selectedItem);
                    cout << "Grabbed: " << selectedItem.name << "\n";
                    rooms[currentRoom].items.erase(rooms[currentRoom].items.begin() + choice - 1);
                }
                return;
            }

            // Handle taking by name
            if (inventory.size() >= MAX_INVENTORY) {
                cout << "Your inventory is full! Drop something first.\n";
                return;
            }

            // Convert input to lowercase for comparison
            string lowerInput = itemName;
            for (char& c : lowerInput) {
                c = tolower(c);
            }

            for (int i = 0; i < rooms[currentRoom].items.size(); i++) {
                string lowerItemName = rooms[currentRoom].items[i].name;
                for (char& c : lowerItemName) {
                    c = tolower(c);
                }

                if (lowerItemName == lowerInput) {
                    if (rooms[currentRoom].items[i].name == "Pressure Gauge") {
                        cout << "The pressure gauge is securely mounted to the wall.\n";
                        return;
                    }
                    inventory.push_back(rooms[currentRoom].items[i]);
                    cout << "Grabbed: " << rooms[currentRoom].items[i].name << "\n";
                    rooms[currentRoom].items.erase(rooms[currentRoom].items.begin() + i);
                    return;
                }
            }
            cout << "You don't see that here.\n";
        }

        void examineItem(string itemName) {
            clearScreen();
            
            // Special case for pressure gauge in airlock
            if (currentRoom == 0 && (itemName == "Pressure Gauge" || itemName == "gauge" || itemName == "pressure")) {
                cout << "\nThe digital display shows critical readings:\n";
                cout << "Main Hull: 68% nominal pressure\n";
                cout << "Deck 2: WARNING - Pressure dropping\n";
                cout << "Life Support: CRITICAL - System malfunction\n";
                cout << "The gauge's warning light pulses an angry red.\n";
                return;
            }

            // If no item specified, show numbered list
            if (itemName.empty()) {
            if (inventory.empty()) {
                    cout << "You have nothing to examine.\n";
                return;
            }

                cout << "What would you like to examine?\n\n";
                
                // Show inventory items
                for (int i = 0; i < inventory.size(); i++) {
                    cout << i + 1 << ". " << inventory[i].name << "\n";
                }
                
                cout << "\nEnter number (or 0 to cancel): ";
            int choice;
                if (!getNumericInput(choice, inventory.size())) {
                    cout << "Invalid input. Please enter a number between 0 and " << inventory.size() << ".\n";
                    return;
                }

            if (choice > 0 && choice <= inventory.size()) {
                    inventory[choice - 1].examine(this);
                }
                return;
            }

            // Handle examining by name
            for (Item& item : inventory) {
                string lowerInput = itemName;
                string lowerItemName = item.name;
                for (char& c : lowerInput) c = tolower(c);
                for (char& c : lowerItemName) c = tolower(c);
                
                if (lowerItemName == lowerInput) {
                    item.examine(this);
                    return;
                }
            }
            cout << "You don't have that item in your inventory.\n";
        }

        void useItem(string itemName) {
            clearScreen();
            
            // If no item specified, show numbered list
            if (itemName.empty()) {
                if (inventory.empty() && (!roomSearched[currentRoom] || (currentRoom != 1 && currentRoom != 2))) {
                    cout << "You have no items to use.\n";
                    return;
                }
                
                cout << "Which item do you want to use?\n\n";
                vector<string> options;
                
                // Add inventory items
                for (const Item& item : inventory) {
                    options.push_back(item.name);
                }
                
                // Add terminals if room is searched
                if (roomSearched[currentRoom]) {
                    if (currentRoom == 1) {  // Maintenance Corridor
                        options.push_back("Life Support System Terminal");
                        options.push_back("Observation Deck Security Terminal");
                    }
                    else if (currentRoom == 2) {  // Observation Deck
                        options.push_back("Navigation System Terminal");
                        options.push_back("Mess Hall Security Terminal");
                    }
                    else if (currentRoom == 4) {  // Control Room
                        options.push_back("Main Computer System Terminal");
                    }
                }

                // Display all options
                for (int i = 0; i < options.size(); i++) {
                    cout << i + 1 << ". " << options[i] << "\n";
                }
                
                cout << "\nEnter number (or 0 to cancel): ";
                int choice;
                if (!getNumericInput(choice, options.size())) {
                    cout << "Invalid input. Please enter a number between 0 and " << options.size() << ".\n";
                    return;
                }
                
                clearScreen();
                if (choice > 0 && choice <= options.size()) {
                    string selectedItem = options[choice - 1];
                    
                    if (selectedItem == "Life Support System Terminal") {
                        examineSystem("life support");
                    }
                    else if (selectedItem == "Navigation System Terminal") {
                        examineSystem("navigation");
                    }
                    else if (selectedItem == "Observation Deck Security Terminal") {
                        // Add suit damage check here first
                        if (!suitDamaged && !suitRepaired && currentRoom == 1) {  // In corridor and suit not damaged yet
                            suitDamaged = true;
                            clearScreen();
                            wrapText("\nAs you reach for the terminal controls, your suit catches on a jagged piece of torn metal!", false, "alert");
                            cout << "\n";
                            wrapText("WARNING: Suit integrity compromised. Oxygen leak detected. Estimated 5 minutes of breathable air remaining.", false, "alert");
                            cout << "\n";
                            wrapText("You need to seal the tear quickly!", false, "alert");
                            cout << "\n";
                            return;
                        }

                        clearScreen();
                        terminalEffect("\n=== OBSERVATION DECK SECURITY TERMINAL ===\n");
                        terminalEffect("Accessing security systems...");
                        terminalEffect("Initiating authentication protocol...\n");
                        
                        cout << "\nEnter security code (or 0 to cancel): ";
                        string input;
                        getline(cin, input);
                        
                        if (input == "0") {
                            terminalEffect("Terminal session terminated.");
                            return;
                        }
                        
                        terminalEffect("Validating code...");
                        usleep(1000000);  // 1 second pause
                        
                        if (input == DOOR_CODE) {
                            terminalEffect("ACCESS GRANTED", 100000);
                            terminalEffect("Disengaging security locks...");
                            terminalEffect("Opening observation deck doors...\n");
                            obsdeckDoorUnlocked = true;
                            currentRoom = 2;
                            cout << "\n";
                            wrapText("Current Location: Observation Deck", true, "info");
                            cout << "\n";
                            
                            // Show room info on first entry
                            if (roomFirstVisit[currentRoom]) {
                                showRoomInfo();
                                roomFirstVisit[currentRoom] = false;
                            }
                        } else {
                            terminalEffect("ACCESS DENIED", 100000);
                            terminalEffect("Invalid security code. Terminal locked for 5 seconds.");
                            for (int i = 5; i > 0; i--) {
                                cout << i << "..." << flush;
                                usleep(1000000);
                            }
                            cout << "\n";
                        }
                        return;
                    }
                    else if (selectedItem == "Mess Hall Security Terminal") {
                        clearScreen();
                        terminalEffect("\n=== MESS HALL SECURITY TERMINAL ===\n");
                        terminalEffect("Accessing security systems...");
                        terminalEffect("Initiating authentication protocol...\n");
                        
                        cout << "\nEnter security code (or 0 to cancel): ";
                        string input;
                        getline(cin, input);
                        
                        if (input == "0") {
                            terminalEffect("Terminal session terminated.");
                            return;
                        }
                        
                        terminalEffect("Validating code...");
                        usleep(1000000);  // 1 second pause
                        
                        if (input == DOOR_CODE) {
                            terminalEffect("ACCESS GRANTED", 100000);
                            terminalEffect("Disengaging security locks...");
                            terminalEffect("Opening mess hall doors...\n");
                            currentRoom = 3;
                            cout << "\n";
                            wrapText("Current Location: Mess Hall", true, "info");
                            cout << "\n";
                            
                            // Show room info on first entry
                            if (roomFirstVisit[currentRoom]) {
                                showRoomInfo();
                                roomFirstVisit[currentRoom] = false;
                            }
                        } else {
                            terminalEffect("ACCESS DENIED", 100000);
                            terminalEffect("Invalid security code. Terminal locked for 5 seconds.");
                            for (int i = 5; i > 0; i--) {
                                cout << i << "..." << flush;
                                usleep(1000000);
                            }
                            cout << "\n";
                        }
                        return;
                    }
                    else if (selectedItem == "Main Computer System Terminal") {
                        examineSystem("computer");
                    }
                    else {
                        // Handle regular inventory items
                        if (selectedItem == "Headlight") {
                            if (hasLight) {
                                cout << "The headlight is already on.\n";
                            }
                            else if (messHallCounterStarted) {  // If we've entered mess hall, batteries are dead
                                cout << "The headlight's batteries are dead.\n";
                            }
                            else {
                                hasLight = true;
                                cout << "You turn on the headlight. The area is illuminated!\n";
                            }
                        }
                        else if (selectedItem == "Radio") {
                            wrapText("You activate the radio, but hear only static. The emergency channels are silent.", false);
                            cout << "\n";
                            wrapText("After a moment, you catch what sounds like a distant signal, but it fades into white noise.", false);
                            cout << "\n";
                        }
                        else if (selectedItem == "Glow Stick" && !hasLight) {
                            hasLight = true;
                            cout << "You crack the glow stick. A green light fills the area!\n";
                        }
                        else if (selectedItem == "Crowbar" && currentRoom == 0) {
                            airlockDoorOpen = true;
                            cout << "You use the crowbar to pry open the airlock door.\n";
                        }
                        else if (selectedItem == "Spare Batteries" && !hasLight && actionCounter >= 15) {
                            hasLight = true;
                            actionCounter = 0;  // Reset counter
                            wrapText("You replace the dead batteries in your headlight. The beam springs back to life!", false);
                            // Remove batteries after use
                            for (int i = 0; i < inventory.size(); i++) {
                                if (inventory[i].name == "Spare Batteries") {
                                    inventory.erase(inventory.begin() + i);
                                    break;
                                }
                            }
                        }
                        else if (selectedItem == "Duct Tape" && suitDamaged && !suitRepaired) {
                            suitRepaired = true;
                            suitDamaged = false;
                            wrapText("You quickly apply the duct tape to seal the tear in your suit. The oxygen leak stops.", false);
                            cout << "\n";
                            wrapText("It's not pretty, but it'll hold.", false);
                        }
                        else if (selectedItem == "Wire Cutters") {
                            if (currentRoom == 1) {  // Maintenance Corridor
                                wrapText("You carefully cut and clear away the loose, sparking wires. The corridor seems a bit safer now.", false);
                                cout << "\n";
                            } else {
                                wrapText("There are no exposed wires that need cutting here.", false);
                                cout << "\n";
                            }
                        }
                        else if (selectedItem == "9V Batteries" && !hasLight && messHallCounter >= 3) {
                            hasLight = true;
                            messHallCounter = 0;  // Reset counter
                            wrapText("You replace the dead batteries in your headlight. The beam springs back to life!", false);
                            cout << "\n";
                        }
                        else if (selectedItem == "Energy Bar") {
                            clearScreen();
                            wrapText("You begin to remove your helmet to eat the energy bar...", false);
                            cout << "\n\n";
                            usleep(2000000);  // 2 second dramatic pause
                            wrapText("The moment you break the helmet seal, warning lights flash on your suit display.", false, "alert");
                            cout << "\n\n";
                            wrapText("What would you like to do?", false);
                            cout << "\n\n";
                            cout << "1. Continue removing helmet to eat the energy bar\n";
                            cout << "2. Quickly reseal your helmet\n";
                            cout << "\nEnter choice: ";
                            
                            int choice;
                            if (!getNumericInput(choice, 2)) {
                                wrapText("You fumble with the helmet, managing to reseal it just in time.", false);
                                cout << "\n";
                                return;
                            }
                            
                            if (choice == 1) {
                                clearScreen();
                                wrapText("You remove your helmet completely...", false);
                                cout << "\n\n";
                                usleep(2000000);
                                wrapText("The thin, toxic atmosphere burns your lungs as you gasp for breath.", false, "alert");
                                cout << "\n";
                                wrapText("With life support offline, the station's air is unbreathable. Your vision begins to blur as oxygen deprivation sets in...", false, "alert");
                                cout << "\n\n";
                                wrapText("You collapse to the floor. The energy bar falls from your lifeless hand.", false);
                                cout << "\n\n";
                                wrapText("GAME OVER", false, "alert");
                                exit(0);
                            } else {
                                clearScreen();
                                wrapText("You quickly attempt to reseal your helmet...", false);
                                cout << "\n\n";
                                usleep(2000000);
                                wrapText("WARNING: Suit oxygen levels at 0%. Seal integrity compromised.", false, "alert");
                                cout << "\n\n";
                                wrapText("Your suit's O2 gauge rapidly drops to zero. The room spins as you desperately try to breathe...", false);
                                cout << "\n\n";
                                wrapText("You collapse, suffocating in your own suit.", false);
                                cout << "\n\n";
                                wrapText("GAME OVER", false, "alert");
                                exit(0);
                            }
                        }
                        else if (selectedItem == "Blow Torch") {
                            if (currentRoom != 3) {  // If not in mess hall
                                wrapText("There's nothing here that needs cutting.", false);
                                cout << "\n";
                            } else {  // In mess hall
                                bool hasButane = false;
                                for (const Item& item : inventory) {
                                    if (item.name == "Butane Canister") {
                                        hasButane = true;
                                        break;
                                    }
                                }
                                
                                if (!hasButane) {
                                    wrapText("Needs fuel to work.", false);
                                    cout << "\n";
                                } else {
                                    controlRoomDoorOpen = true;
                                    wrapText("You attach the butane canister to the blow torch and cut through the control room door's emergency locks.", false);
                                    cout << "\n";
                                    wrapText("The way to the control room is now clear.", false);
                                    cout << "\n";
                                    
                                    // Remove butane canister after use
                                    for (int i = 0; i < inventory.size(); i++) {
                                        if (inventory[i].name == "Butane Canister") {
                                            inventory.erase(inventory.begin() + i);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        else {
                            cout << "You can't use that here.\n";
                        }
                    }
                }
                return;
            }

            // Convert itemName to lowercase
            string lowerItemName = itemName;
            for (char& c : lowerItemName) {
                c = tolower(c);
            }

            for (Item& item : inventory) {
                // Convert inventory item name to lowercase for comparison
                string lowerInvItem = item.name;
                for (char& c : lowerInvItem) {
                    c = tolower(c);
                }
                
                if (lowerInvItem == lowerItemName) {
                    if (item.name == "Headlight") {
                        if (hasLight) {
                            cout << "The headlight is already on.\n";
                        }
                        else if (messHallCounterStarted) {  // If we've entered mess hall, batteries are dead
                            cout << "The headlight's batteries are dead.\n";
                        }
                        else {
                            hasLight = true;
                            cout << "You turn on the headlight. The area is illuminated!\n";
                        }
                    }
                    else if (item.name == "Radio") {
                        wrapText("You activate the radio, but hear only static. The emergency channels are silent.", false);
                        cout << "\n";
                        wrapText("After a moment, you catch what sounds like a distant signal, but it fades into white noise.", false);
                        cout << "\n";
                    }
                    else if (item.name == "Glow Stick" && !hasLight) {
                        hasLight = true;
                        cout << "You crack the glow stick. A green light fills the area!\n";
                    }
                    else if (item.name == "Crowbar" && currentRoom == 0) {
                        airlockDoorOpen = true;
                        cout << "You use the crowbar to pry open the airlock door.\n";
                    }
                    else if (item.name == "Spare Batteries" && !hasLight && actionCounter >= 15) {
                        hasLight = true;
                        actionCounter = 0;  // Reset counter
                        wrapText("You replace the dead batteries in your headlight. The beam springs back to life!", false);
                        // Remove batteries after use
                        for (int i = 0; i < inventory.size(); i++) {
                            if (inventory[i].name == "Spare Batteries") {
                                inventory.erase(inventory.begin() + i);
                                break;
                            }
                        }
                    }
                    else if (item.name == "Duct Tape" && suitDamaged && !suitRepaired) {
                        suitRepaired = true;
                        suitDamaged = false;
                        wrapText("You quickly apply the duct tape to seal the tear in your suit. The oxygen leak stops.", false);
                        cout << "\n";
                        wrapText("It's not pretty, but it'll hold.", false);
                    }
                    else if (item.name == "Wire Cutters") {
                        if (currentRoom == 1) {  // Maintenance Corridor
                            wrapText("You carefully cut and clear away the loose, sparking wires. The corridor seems a bit safer now.", false);
                            cout << "\n";
                        } else {
                            wrapText("There are no exposed wires that need cutting here.", false);
                            cout << "\n";
                        }
                    }
                    else if (item.name == "9V Batteries" && !hasLight && messHallCounter >= 3) {
                        hasLight = true;
                        messHallCounter = 0;  // Reset counter
                        wrapText("You replace the dead batteries in your headlight. The beam springs back to life!", false);
                        cout << "\n";
                    }
                    else if (item.name == "Energy Bar") {
                        clearScreen();
                        wrapText("You begin to remove your helmet to eat the energy bar...", false);
                        cout << "\n\n";
                        usleep(2000000);  // 2 second dramatic pause
                        wrapText("The moment you break the helmet seal, warning lights flash on your suit display.", false, "alert");
                        cout << "\n\n";
                        wrapText("What would you like to do?", false);
                        cout << "\n\n";
                        cout << "1. Continue removing helmet to eat the energy bar\n";
                        cout << "2. Quickly reseal your helmet\n";
                        cout << "\nEnter choice: ";
                        
                        int choice;
                        if (!getNumericInput(choice, 2)) {
                            wrapText("You fumble with the helmet, managing to reseal it just in time.", false);
                            cout << "\n";
                            return;
                        }
                        
                        if (choice == 1) {
                            clearScreen();
                            wrapText("You remove your helmet completely...", false);
                            cout << "\n\n";
                            usleep(2000000);
                            wrapText("The thin, toxic atmosphere burns your lungs as you gasp for breath.", false, "alert");
                            cout << "\n";
                            wrapText("With life support offline, the station's air is unbreathable. Your vision begins to blur as oxygen deprivation sets in...", false, "alert");
                            cout << "\n\n";
                            wrapText("You collapse to the floor. The energy bar falls from your lifeless hand.", false);
                            cout << "\n\n";
                            wrapText("GAME OVER", false, "alert");
                            exit(0);
                        } else {
                            clearScreen();
                            wrapText("You quickly attempt to reseal your helmet...", false);
                            cout << "\n\n";
                            usleep(2000000);
                            wrapText("WARNING: Suit oxygen levels at 0%. Seal integrity compromised.", false, "alert");
                            cout << "\n\n";
                            wrapText("Your suit's O2 gauge rapidly drops to zero. The room spins as you desperately try to breathe...", false);
                            cout << "\n\n";
                            wrapText("You collapse, suffocating in your own suit.", false);
                            cout << "\n\n";
                            wrapText("GAME OVER", false, "alert");
                            exit(0);
                        }
                    }
                    else {
                        cout << "You can't use that here.\n";
                    }
                    return;
                }
            }
            cout << "You don't have that item.\n";
        }

        void dropItem(string itemName) {
            clearScreen();
            
            // If no item specified, show numbered list
            if (itemName.empty()) {
                if (inventory.empty()) {
                    cout << "You have no items to drop.\n";
                    return;
                }

                cout << "What do you want to drop?\n\n";
                for (int i = 0; i < inventory.size(); i++) {
                    cout << i + 1 << ". " << inventory[i].name << "\n";
                }
                
                cout << "\nEnter number (or 0 to cancel): ";
                int choice;
                if (!getNumericInput(choice, inventory.size())) {
                    cout << "Invalid input. Please enter a number between 0 and " << inventory.size() << ".\n";
                    return;
                }
                
                clearScreen();
                if (choice > 0 && choice <= inventory.size()) {
                    // Check if trying to drop headlight in dark area
                    if (inventory[choice - 1].name == "Headlight" && !hasGlowStickLight && (currentRoom == 0 || currentRoom == 1)) {
                        wrapText("You can't drop your only light source in a dark area!", false, "alert");
                        cout << "\n";
                        return;
                    }
                    
                    rooms[currentRoom].items.push_back(inventory[choice - 1]);
                    cout << "Dropped: " << inventory[choice - 1].name << "\n";
                    inventory.erase(inventory.begin() + choice - 1);
                    return;
                }
                return;
            }

            // Handle dropping by name
            for (int i = 0; i < inventory.size(); i++) {
                if (inventory[i].name == itemName) {
                    // Check if trying to drop headlight in dark area
                    if (itemName == "Headlight" && !hasGlowStickLight && (currentRoom == 0 || currentRoom == 1)) {
                        wrapText("You can't drop your only light source in a dark area!", false, "alert");
                        cout << "\n";
                        return;
                    }
                    
                    rooms[currentRoom].items.push_back(inventory[i]);
                    cout << "Dropped: " << itemName << "\n";
                    inventory.erase(inventory.begin() + i);
                    return;
                }
            }
            cout << "You don't have that item.\n";
        }

        void showMap() {
            clearScreen();
            cout << "\n=== Station Layout & Mission Info ===\n\n";
            
            string map[] = {
                "   ╔══════════════╗",
                "   ║ Control Room ║",
                "   ╚══════╦═══════╝",
                "          ║      ",
                "    ╔═════╩═════╗",
                "    ║ Mess Hall ║",
                "    ╚═════╦═════╝",
                "          ║      ",
                " ╔════════╩═════════╗",
                " ║ Observation Deck ║",
                " ╚════════╦═════════╝",
                "          ║      ",
                "    ╔═════╩═════╗",
                "    ║  Corridor ║",
                "    ╚═════╦═════╝",
                "          ║      ",
                "    ╔═════╩═════╗",
                "    ║  Airlock  ║",
                "    ╚═══════════╝"
            };

            // Show map with single centered arrow for current location
            for (int i = 0; i < 19; i++) {
                if (i/2 == 8-currentRoom*2) {
                    cout << "           " << map[i] << "\n";
                    cout << "      -->  " << map[i+1] << "\n";
                    i++;
                } else {
                    cout << "           " << map[i] << "\n";
                }
            }

            wrapText("=== Mission Objectives ===", false);
            cout << "\n";
            wrapText("1. Make your way through the space station to reach the Control Room", true);
            wrapText("2. Collect necessary repair tools and equipment", true);
            wrapText("3. Restore all critical systems", true);  // Simplified objective
        }

        void examineRoomItem() {
            clearScreen();
            
            // Check for light in dark rooms first
            if (!hasLight && (currentRoom == 0 || currentRoom == 1)) {
                wrapText("The room is too dark to make out any details. You'll need a light source first.", false, "alert");
                cout << "\n";
                return;
            }

            // Check if room has been searched
            if (!roomSearched[currentRoom]) {
                wrapText("You should search the room first to find anything worth examining.", false, "info");
                cout << "\n";
                return;
            }

            cout << "\nWhat would you like to examine?\n\n";

            // Show regular items
            vector<string> options;
            for (const Item& item : rooms[currentRoom].items) {
                options.push_back(item.name);
            }

            // Add terminals only after room has been searched
            if (currentRoom == 1) {
                options.push_back("Life Support System Terminal");
                options.push_back("Observation Deck Security Terminal");
            }
            if (currentRoom == 2) {
                options.push_back("Navigation System Terminal");
                options.push_back("Mess Hall Security Terminal");
            }
            if (currentRoom == 4) {  // Add Control Room terminal
                options.push_back("Main Computer System Terminal");
            }

            // Display all options
            for (int i = 0; i < options.size(); i++) {
                cout << i + 1 << ". " << options[i] << "\n";
            }

            int choice;
            if (!getNumericInput(choice, options.size())) {
                cout << "Invalid input. Please enter a number between 0 and " << options.size() << ".\n";
                return;
            }

            if (choice > 0 && choice <= options.size()) {
                if (options[choice - 1] == "Life Support System Terminal") {
                    examineSystem("life support");
                }
                else if (options[choice - 1] == "Navigation System Terminal") {
                    examineSystem("navigation");
                }
                else if (options[choice - 1] == "Main Computer System Terminal") {
                    examineSystem("computer");
                }
                else {
                    for (Item& item : rooms[currentRoom].items) {
                        if (item.name == options[choice - 1]) {
                            item.examine(this);
                            break;
                        }
                    }
                }
            }
        }

        void clearScreen() {
            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif
        }

        void showHelp() {
            wrapText("Available Commands:", false);
            cout << "\n";
            wrapText("- search (s)", true);
            wrapText("- view inventory (I)", true);
            wrapText("- info (i, room info)", true);
            wrapText("- grab [item] (g)", true);
            wrapText("- examine [item] (e)", true);
            wrapText("- use [item] (u)", true);
            wrapText("- drop [item] (d)", true);
            wrapText("- move rooms (M, Move, open door)", true);
            wrapText("- show map/map (m)", true);
            wrapText("- help (h)", true);
            wrapText("- quit (q)", true);
        }

        void showRoomInfo() {
            clearScreen();
            cout << "\n=== " << rooms[currentRoom].name << " Information ===\n\n";
            
            switch(currentRoom) {
                case 0:
                    wrapText("The airlock serves as the primary entry and exit point for the station. The reinforced doors are designed to withstand extreme pressure differences.", false);
                    cout << "\n";
                    wrapText("CAUTION: Emergency lighting systems are non-functional.", false, "alert");
                    cout << "\n";
                    break;
                case 1:
                    wrapText("The maintenance corridor houses the station's vital infrastructure. Power conduits and life support systems run through its walls.", false);
                    cout << "\n";
                    wrapText("Engineering Note: Last scheduled maintenance was interrupted mid-task. Tools left behind suggest a hasty evacuation.", false, "info");
                    cout << "\n";
                    wrapText("CAUTION: Unstable power fluctuations detected in primary conduits.", false, "alert");
                    cout << "\n";
                    break;
                case 2:
                    wrapText("The observation deck's reinforced windows provide a 180-degree view of space.", false);
                    cout << "\n";
                    wrapText("Log Entry: Strange readings were reported by the night shift. Several instruments show impossible stellar configurations.", false, "info");
                    cout << "\n";
                    wrapText("Status: Backup navigation systems are operational but reporting conflicting coordinates.", false, "alert");
                    cout << "\n";
                    break;
                case 3:
                    wrapText("The mess hall was designed for a crew of twelve. Food synthesizers and storage units line the walls.", false);
                    cout << "\n";
                    wrapText("Personal Log: 'The coffee machine started making strange noises this morning. Then all hell broke loose.'", false, "info");
                    cout << "\n";
                    break;
                case 4:
                    wrapText("The control room is the brain of the station. All critical systems can be monitored and controlled from here.", false);
                    cout << "\n";
                    wrapText("Final Log: 'Multiple system failures detected. Navigation errors increasing. Emergency protocols initiated.'", false, "info");
                    cout << "\n";
                    wrapText("CRITICAL: Main computer core experiencing cascading failures.", false, "alert");
                    cout << "\n";
                    break;
            }
        }

        // Add this helper function to Game class
        void terminalEffect(const string& text, int delay = 30000) {
            for (char c : text) {
                cout << c << flush;
                usleep(delay);  // Microseconds delay between characters
            }
            cout << endl;
        }

        void examineSystem(string systemName) {
            clearScreen();
            
            if (systemName == "computer" && currentRoom == 4) {  // Control Room
                clearScreen();
                wrapText("The computer appears to be malfunctioning. It only displays ascii characters in hexadecimal format. You can try to access it.", false);
                cout << "\n\n";
                terminalEffect("=== MAIN COMPUTER DIAGNOSTIC TERMINAL ===\n");
                
                if (!computerSystemFixed) {
                    terminalEffect("70 61 73 73 77 6F 72 64 3A", 20000);  // "Password:" in hex
                    cout << "\n\n";
                    string input;
                    getline(cin, input);
                    cout << "\n";

                    // Convert any typed letters to their hex values
                    string hexInput = "";
                    for (char c : input) {
                        if (isalpha(c)) {
                            char upperC = toupper(c);
                            char hex[3];
                            sprintf(hex, "%02X", upperC);
                            hexInput += hex;
                            hexInput += " ";
                        } else if (c == ' ') {
                            hexInput += c;
                        } else {
                            hexInput += c;
                        }
                    }
                    
                    if (input == "70617373776F7264" || input == "70 61 73 73 77 6F 72 64") {  // hex for "password"
                        terminalEffect("41 63 63 65 73 73 20 47 72 61 6E 74 65 64", 50000);  // "Access Granted"
                        
                        if (hasItem("Circuit Board")) {
                            clearScreen();
                            terminalEffect("\n=== MAIN COMPUTER DIAGNOSTIC TERMINAL ===\n\n");
                            
                            terminalEffect(">> Initiating diagnostic scan...", 30000);
                            terminalEffect(">> Analyzing system architecture...", 30000);
                            terminalEffect(">> Fault detected: Primary circuit board malfunction", 30000);
                            cout << "\n";
                            
                            terminalEffect(">> Removing damaged component...", 30000);
                            terminalEffect(">> Installing replacement circuit board...", 30000);
                            terminalEffect(">> Verifying new hardware...", 30000);
                            cout << "\n";
                            
                            terminalEffect(">> System restoration in progress...", 30000);
                            terminalEffect(">> Power grid stabilizing...", 30000);
                            terminalEffect(">> Station systems coming online...", 30000);
                            cout << "\n";
                            
                            terminalEffect(">> All systems operational", 30000);
                            terminalEffect(">> Station functionality restored to 100%", 30000);
                            cout << "\n\n";
                            
                            wrapText("Congratulations! You've successfully restored the station's systems!", false, "alert");
                            wrapText("The space station will now resume normal operations.", false);
                            cout << "\n";
                            wrapText("Thank you for playing!", false, "info");  // Add this line
                            cout << "\n";
                            wrapText("Press Enter to end session...", false);
                            cin.get();
                            exit(0);
                        } else {
                            terminalEffect("52 75 6E 6E 69 6E 67 20 44 69 61 67 6E 6F 73 74 69 63", 50000);  // "Running Diagnostic"
                            terminalEffect("43 6F 6D 70 75 74 65 72 20 43 6F 6D 70 6F 6E 65 6E 74 20 4D 61 6C 66 75 6E 63 74 69 6F 6E 69 6E 67", 50000);  // "Computer Component Malfunctioning"
                        }
                    } else {
                        terminalEffect("41 63 63 65 73 73 20 44 65 6E 69 65 64", 50000);  // "Access Denied"
                        terminalEffect("45 72 72 6F 72 20 2D 20 49 6E 76 61 6C 69 64 20 48 65 78 20 43 6F 64 65", 50000);  // "Error - Invalid Hex Code"
                    }
                } else {
                    wrapText("The computer system is functioning normally.", true);
                }
            }
            else if (systemName == "navigation" && currentRoom == 2) {  // Observation Deck
                terminalEffect("\n=== NAVIGATION SYSTEM TERMINAL ===\n");
                terminalEffect("Accessing navigation controls...");
                terminalEffect("Checking system status...\n");
                
                if (!computerSystemFixed) {
                    terminalEffect("ERROR: Cannot establish connection to main computer", 50000);
                    wrapText("Navigation system is locked out. Main computer must be repaired first.", false, "alert");
                    cout << "\n";  // Add newline after error message
                }
                else if (!navigationSystemFixed) {
                    if (hasRequiredTools("navigation")) {
                        terminalEffect("Initiating calibration sequence...");
                        terminalEffect("Aligning stellar sensors...");
                        terminalEffect("Updating star charts...");
                        wrapText("You successfully calibrate and repair the navigation system.", false);
                        navigationSystemFixed = true;
                        terminalEffect("Navigation systems restored");
                        terminalEffect("Current Position: Sector 7, Quadrant 3");
                        terminalEffect("All tracking systems: Online");
                        terminalEffect("Course plotting: Available");
                    } else {
                        wrapText("The navigation system needs repairs. You'll need a Star Chart and Telescope Lens.", false, "info");
                    }
                } else {
                    terminalEffect("Navigation systems operating normally");
                    terminalEffect("Current Position: Sector 7, Quadrant 3");
                    terminalEffect("All tracking systems: Online");
                    terminalEffect("Course plotting: Available");
                }
            }
            else if (systemName == "life support" && currentRoom == 1) {  // Maintenance
                terminalEffect("\n=== LIFE SUPPORT CONTROL TERMINAL ===\n");
                terminalEffect("Accessing life support controls...");
                terminalEffect("Checking system status...\n");
                
                if (!computerSystemFixed) {
                    terminalEffect("ERROR: Cannot establish connection to main computer", 50000);
                    wrapText("Life Support system is locked out. Main computer must be repaired first.", true, "alert");
                }
                else if (!lifeSupportFixed) {
                    if (hasRequiredTools("life support")) {
                        terminalEffect("Initiating repair sequence...");
                        terminalEffect("Installing water filtration system...");
                        terminalEffect("Connecting backup power...");
                        wrapText("You manage to repair and stabilize the life support system.", true);
                        lifeSupportFixed = true;
                        terminalEffect("Life support systems operating at optimal levels");
                        terminalEffect("Oxygen levels: Normal");
                        terminalEffect("Pressure: Stable");
                        terminalEffect("Temperature: 21°C");
                    } else {
                        wrapText("The life support system requires repairs. You'll need a Water Filter and Battery Pack.", true, "info");
                    }
                } else {
                    terminalEffect("Life support systems operating at optimal levels");
                    terminalEffect("Oxygen levels: Normal");
                    terminalEffect("Pressure: Stable");
                    terminalEffect("Temperature: 21°C");
                }
            }
            cout << "\n";
        }

        bool hasRequiredTools(string system) {
            if (system == "computer") {
                return hasItem("Circuit Board") && hasItem("Codex");
            }
            else if (system == "navigation") {
                return hasItem("Star Chart") && hasItem("Telescope Lens");
            }
            else if (system == "life support") {
                return hasItem("Water Filter") && hasItem("Battery Pack");
            }
            return false;
        }

        bool hasItem(string itemName) {
            for (const Item& item : inventory) {
                if (item.name == itemName) return true;
            }
            return false;
        }

        void displayManualText(const string& text) {
            wrapText(text, false);
            cout << "\n";
        }

        // Add this helper function to Game class
        bool getNumericInput(int& choice, int maxChoice) {
            string input;
            getline(cin, input);
            
            try {
                if (input.empty()) {
                    return false;
                }
                choice = stoi(input);
                if (choice < 0 || choice > maxChoice) {
                    return false;
                }
                return true;
            }
            catch (...) {
                return false;
            }
        }

        bool hasLightSource() {
            return hasLight || hasGlowStickLight;  // Return true if either light source is active
        }

        // Add new function to Game class
        void feelAround() {
            if (!hasLightSource() && !feelAroundUsed && !rooms[currentRoom].items.empty()) {
                // 50% chance to find an item
                if (rand() % 2 == 0) {
                    // Pick random item from room
                    int randomIndex = rand() % rooms[currentRoom].items.size();
                    Item foundItem = rooms[currentRoom].items[randomIndex];
                    
                    if (inventory.size() >= MAX_INVENTORY) {
                        wrapText("You found something in the darkness, but your inventory is full!", false);
                        cout << "\n";
                        return;
                    }
                    
                    wrapText("Feeling around in the darkness, your hand touches something...", false);
                    cout << "\n";
                    inventory.push_back(foundItem);
                    rooms[currentRoom].items.erase(rooms[currentRoom].items.begin() + randomIndex);
                    wrapText("You found: " + foundItem.name, false);
                    cout << "\n";
                } else {
                    wrapText("You feel around in the darkness but find nothing useful.", false);
                    cout << "\n";
                }
                feelAroundUsed = true;
            } else if (hasLightSource()) {
                wrapText("You can see clearly with your light source. Try searching instead.", false);
                cout << "\n";
            } else if (feelAroundUsed) {
                wrapText("You've already thoroughly felt around this area.", false);
                cout << "\n";
            } else {
                wrapText("You feel around but find nothing.", false);
                cout << "\n";
            }
        }
};

// Add the examine implementation after Game class is defined
void Item::examine(Game* game) {
    game->clearScreen();  // Clear screen before showing item details
    
    if (name == "Repair Manual") {
        game->wrapText("Item: " + name, false);
        cout << "\n";
        game->wrapText("A technical manual detailing station systems. Several pages are bookmarked:", false);
        cout << "\n";
        
        game->wrapText("CRITICAL SYSTEMS STATUS:", false);
        game->wrapText("1. Life Support System", true);
        game->wrapText("- Chemical imbalance detected in O2 recycling", true);
        game->wrapText("- O2/N2 mixture: 17.3% (WARNING: Below safe threshold)", true);
        game->wrapText("- Requires main computer for mixture calibration", true);
        cout << "\n";
        
        game->wrapText("2. Navigation System", true);
        game->wrapText("- Position verification failure", true);
        game->wrapText("- Stellar drift calculation error: -47.3 parsecs", true);
        game->wrapText("- Main computer connection required for triangulation", true);
        cout << "\n";
        
        game->wrapText("3. Computer Core", true);
        game->wrapText("- Primary systems offline", true);
        game->wrapText("- Required for all critical system calibration", true);
        game->wrapText("- Must be repaired first to enable other systems", true);
        cout << "\n";
        
        game->wrapText("WARNING: Attempting system repairs without main computer online may result in cascading failures.", false, "alert");
        cout << "\n";
    } else if (name == "ASCII Table") {  // Changed from "Codex"
        game->wrapText("Item: " + name, false);
        cout << "\n\n";
        
        cout << "=== ASCII HEX REFERENCE ===\n\n";
        cout << "Hex  Char   |  Hex  Char   |  Hex  Char\n";
        cout << "----------------------------|----------\n";
        cout << "41   A      |  4D    M     |  59    Y\n";
        cout << "42   B      |  4E    N     |  5A    Z\n";
        cout << "43   C      |  4F    O     |  20   [space]\n";
        cout << "44   D      |  50    P     |  3A    :\n";
        cout << "45   E      |  51    Q     |  2D    -\n";
        cout << "46   F      |  52    R     |  2E    .\n";
        cout << "47   G      |  53    S     |  2C    ,\n";
        cout << "48   H      |  54    T     |  21    !\n";
        cout << "49   I      |  55    U     |  3F    ?\n";
        cout << "4A   J      |  56    V     |  28    (\n";
        cout << "4B   K      |  57    W     |  29    )\n";
        cout << "4C   L      |  58    X     |  27    '\n";
        cout << "\n";
        cout << "61   a      |  6D    m     |  79    y\n";
        cout << "62   b      |  6E    n     |  7A    z\n";
        cout << "63   c      |  6F    o     |  \n";
        cout << "64   d      |  70    p     |  \n";
        cout << "65   e      |  71    q     |  \n";
        cout << "66   f      |  72    r     |  \n";
        cout << "67   g      |  73    s     |  \n";
        cout << "68   h      |  74    t     |  \n";
        cout << "69   i      |  75    u     |  \n";
        cout << "6A   j      |  76    v     |  \n";
        cout << "6B   k      |  77    w     |  \n";
        cout << "6C   l      |  78    x     |  \n";
        cout << "\n";
    } else {
        game->wrapText("Item: " + name, false);
        cout << "\n\n";
        game->wrapText(description, false);
        cout << "\n";
    }
}

int main() {
    Game game;
    string input;
    
    while (true) {
        cout << "\n> ";  // Add newline before prompt
        getline(cin, input);
        game.parseCommand(input);
    }
    
    return 0;
}