#include <iostream>
#include <cstdlib>

class SkipListNode
{
public:
    int value;
    SkipListNode **next;

    SkipListNode(int value, int level)
    {
        this->value = value;
        this->next = new SkipListNode *[level + 1];
        for (int i = 0; i <= level; i++)
        {
            this->next[i] = nullptr;
        }
    }

    ~SkipListNode()
    {
        delete[] next;
    }
};

class SkipList
{
private:
    int maxLevel;
    int level;
    SkipListNode *head;

    int randomLevel()
    {
        int level = 0;
        while (rand() < RAND_MAX / 2 && level < maxLevel)
        {
            level++;
        }
        return level;
    }

public:
    SkipList(int maxLevel)
    {
        this->maxLevel = maxLevel;
        this->level = 0;
        this->head = new SkipListNode(0, maxLevel);
    }

    ~SkipList()
    {
        SkipListNode *current = head->next[0];
        while (current != nullptr)
        {
            SkipListNode *next = current->next[0];
            delete current;
            current = next;
        }
        delete head;
    }

    void insert(int value)
    {
        SkipListNode *update[maxLevel + 1];
        SkipListNode *current = head;

        for (int i = level; i >= 0; i--)
        {
            while (current->next[i] != nullptr && current->next[i]->value < value)
            {
                current = current->next[i];
            }
            update[i] = current;
        }

        current = current->next[0];

        if (current == nullptr || current->value != value)
        {
            int newLevel = randomLevel();
            if (newLevel > level)
            {
                for (int i = level + 1; i <= newLevel; i++)
                {
                    update[i] = head;
                }
                level = newLevel;
            }

            SkipListNode *newNode = new SkipListNode(value, newLevel);

            for (int i = 0; i <= newLevel; i++)
            {
                newNode->next[i] = update[i]->next[i];
                update[i]->next[i] = newNode;
            }
        }
    }

    void remove(int value)
    {
        SkipListNode *update[maxLevel + 1];
        SkipListNode *current = head;

        for (int i = level; i >= 0; i--)
        {
            while (current->next[i] != nullptr && current->next[i]->value < value)
            {
                current = current->next[i];
            }
            update[i] = current;
        }

        current = current->next[0];

        if (current != nullptr && current->value == value)
        {
            for (int i = 0; i <= level; i++)
            {
                if (update[i]->next[i] != current)
                {
                    break;
                }
                update[i]->next[i] = current->next[i];
            }
            delete current;

            while (level > 0 && head->next[level] == nullptr)
            {
                level--;
            }
        }
    }

    bool search(int value)
    {
        SkipListNode *current = head;
        for (int i = level; i >= 0; i--)
        {
            while (current->next[i] != nullptr && current->next[i]->value < value)
            {
                current = current->next[i];
            }
        }
        current = current->next[0];
        return current != nullptr && current->value == value;
    }

    void display()
    {
        for (int i = level; i >= 0; i--)
        {
            SkipListNode *current = head->next[i];
            std::cout << "Level " << i << ": ";
            while (current != nullptr)
            {
                std::cout << current->value << " ";
                current = current->next[i];
            }
            std::cout << std::endl;
        }
    }
};

int main()
{
    SkipList skipList(4);
    skipList.insert(10);
    skipList.insert(5);
    skipList.insert(15);
    skipList.insert(7);
    skipList.insert(12);
    skipList.insert(3);

    skipList.display();

    std::cout << "Search 7: " << (skipList.search(7) ? "Found" : "Not Found") << std::endl;
    std::cout << "Search 20: " << (skipList.search(20) ? "Found" : "Not Found") << std::endl;

    skipList.remove(7);
    skipList.remove(15);

    skipList.display();

    return 0;
}