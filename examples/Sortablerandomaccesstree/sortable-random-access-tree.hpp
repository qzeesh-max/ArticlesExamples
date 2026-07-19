#ifndef SORTABLE_RANDOM_ACCESS_TREE

#pragma once

#define SORTABLE_RANDOM_ACCESS_TREE

#include "dynamic_comparator_builder.hpp"
#include <iomanip>


template <typename Value>
        class SortableRandomAccessTree
{
private:
    enum child_t
    {
        LEFT,
        RIGHT,
    };

    enum color_t
    {
        BLACK,
        RED,
    };

    struct color_and_count_t
    {
#if __BYTE_ORDER == __BIG_ENDIAN
        color_t Color : 1;
        unsigned int Count : 31;
#else // Assume little endian in other cases
        unsigned int Count : 31;
        color_t Color : 1;
#endif

    };

    struct Node
    {
        color_and_count_t ColorAndCount;
        Node* children[2];
        Node* parent;
        Value value;


        Node()
        {
            ColorAndCount.Color = RED;
            ColorAndCount.Count = 0;
            children[LEFT] = children[RIGHT] = NULL;
            parent = NULL;
        }

        Node(color_t _color,
             Node* _leftChild,
             Node* _rightChild,
             Node* _parent,
             int _count,
             const Value& _value):
        children{_leftChild, _rightChild},
        parent(_parent),
        value(_value)
        {
            ColorAndCount.Color = _color;
            ColorAndCount.Count = _count;
        }


        ~Node()
        {
            if (children[LEFT])
                delete children[LEFT];

            if (children[RIGHT])
                delete children[RIGHT];
        }

        bool IsLeftChild() const
        {
            return parent && (parent->children[LEFT]==this);
        }

        bool IsRightChild() const
        {
            return parent && (parent->children[RIGHT]==this);
        }

        Node * GetGrandParent() const
        {
            return parent ? parent->parent : NULL;
        }

        Node * GetUncle() const
        {
            Node * GrandParent = GetGrandParent();

            if (GrandParent==NULL)
                return NULL;

            // return the sibling of the parent
            return GrandParent->children[parent->IsLeftChild()];
        }

        Node * GetSibling() const
        {
            if (parent==NULL)
                return NULL;

            return parent->children[IsLeftChild()];
        }


        void PivotLeft(Node*& root)
        {
            Node * temp = children[RIGHT];

            int iTopCount = ColorAndCount.Count,
            iMiddleCount = temp->ColorAndCount.Count,
            iBottomCount = temp->children[LEFT] ? temp->children[LEFT]->ColorAndCount.Count : 0;

            children[RIGHT] = temp->children[LEFT];

            if (temp->children[LEFT]!=NULL)
                temp->children[LEFT]->parent = this;

            temp->parent = parent;

            if (temp->parent==NULL)
                root = temp;
            else
                parent->children[!this->IsLeftChild()] = temp;

            temp->children[LEFT] = this;
            parent = temp;

            temp->ColorAndCount.Count = iTopCount;
            ColorAndCount.Count = iTopCount - iMiddleCount + iBottomCount;
        }

        void PivotRight(Node*& root)
        {
            Node * temp = children[LEFT];

            int iTopCount = ColorAndCount.Count,
            iMiddleCount = temp->ColorAndCount.Count,
            iBottomCount = temp->children[RIGHT] ? temp->children[RIGHT]->ColorAndCount.Count : 0;

            children[LEFT] = temp->children[RIGHT];

            if (temp->children[RIGHT]!=NULL)
                temp->children[RIGHT]->parent = this;

            temp->parent = parent;

            if (temp->parent==NULL)
                root = temp;
            else
                parent->children[!this->IsLeftChild()] = temp;

            temp->children[RIGHT] = this;
            parent = temp;

            temp->ColorAndCount.Count = iTopCount;
            ColorAndCount.Count = iTopCount - iMiddleCount + iBottomCount;
	
        }
	
	static void SwapParents(Node * a, Node * b)
	{
		if (a->parent)
		{
			if (b->parent)
			{
				std::swap(a->parent->children[!a->IsLeftChild()], b->parent->children[!b->IsLeftChild()]);
			} else
			{
				a->parent->children[!a->IsLeftChild()] = b;
			}
						
		} else if (b->parent) {
			b->parent->children[!b->IsLeftChild()] = a;
		}
	}


	void SwapRelatives(Node * other)
	{
		int thisParentIndex = !this->IsLeftChild();
		int otherParentIndex = !other->IsLeftChild();
		
		if (parent!=NULL)
			parent->children[thisParentIndex] = other;
		
		other->parent = parent;
		this->parent = other;
		

		Node * thisChild = this->children[!otherParentIndex];
		
		children[LEFT] = other->children[LEFT];
		children[RIGHT] = other->children[RIGHT];
		
		other->children[otherParentIndex] = this;
		
		if (thisChild)
		{
			thisChild->parent = other;
			other->children[!otherParentIndex] = thisChild;
			
		}
		
		if (children[LEFT])
			children[LEFT]->parent = this;
			
		if (children[RIGHT])
			children[RIGHT]->parent = this;
		
		std::swap(ColorAndCount, other->ColorAndCount);		
		
	}
		
	void Swap(Node * other)
	{
		if (other->parent == this)
		{
			this->SwapRelatives(other);
			
			return;
		} else if (this->parent == other)
		{
			other->SwapRelatives(this);
			
			return;
		}
			
		SwapParents(this, other);
		
		if (children[LEFT] && other->children[LEFT])
			std::swap(children[LEFT]->parent, other->children[LEFT]->parent);
		else if (children[LEFT])
			children[LEFT]->parent = other;
		else if (other->children[LEFT])
			other->children[LEFT]->parent = this;
			
		if (children[RIGHT] && other->children[RIGHT])
			std::swap(children[RIGHT]->parent, other->children[RIGHT]->parent);
		else if (children[RIGHT])
			children[RIGHT]->parent = other;
		else if (other->children[RIGHT])
			other->children[RIGHT]->parent = this;
			
			
			
		std::swap(ColorAndCount, other->ColorAndCount);
		std::swap(children[LEFT], other->children[LEFT]);
		std::swap(children[RIGHT], other->children[RIGHT]);
		std::swap(parent, other->parent);
	}

        Node * Minimum()
        {
            Node * iterator = this;

            while (iterator->children[LEFT]!=NULL)
                iterator = iterator->children[LEFT];

            return iterator;
        }

        Node * Maximum()
        {
            Node * iterator = this;

            while (iterator->children[RIGHT]!=NULL)
                iterator = iterator->children[RIGHT];

            return iterator;

        }

        Node * Successor()
        {
            if (children[RIGHT]!=NULL)
                return children[RIGHT]->Minimum();

            Node * iterator = parent;
            Node * node = this;

            while ((iterator!=NULL) && (node==iterator->children[RIGHT]))
            {
                node = iterator;
                iterator = iterator->parent;
            }

            return iterator;

        }


        Node * Predecessor()
        {
            if (children[LEFT]!=NULL)
                return children[LEFT]->Maximum();

            Node * iterator = parent;
            Node * node = this;

            while ((iterator!=NULL) && (node==iterator->children[LEFT]))
            {
                node = iterator;
                iterator = iterator->parent;
            }

            return iterator;

        }


    };

    Node * root, * leftMost, *rightMost;
    DynamicComparatorBuilder<Value> comparator;
    
private: 
    // We are disabling the copy constructor, moving constructor, assignment operator, and the moving operator for scope of this implementation
    SortableRandomAccessTree(const SortableRandomAccessTree&) = delete;
    SortableRandomAccessTree(const SortableRandomAccessTree&&) = delete;
    
    SortableRandomAccessTree& operator=(const SortableRandomAccessTree&) = delete;
    SortableRandomAccessTree& operator=(const SortableRandomAccessTree&&) = delete;

private:
    // Basic BST Tree insertion, we only need this to be a static function
    void Insert(Node *& _root, const Value& _value, Node * _parent, Node*& _created)
    {
        if (_root==NULL)
        {
            _created = _root = new Node(RED, NULL, NULL, _parent, 1, _value);

            return;
        }


        Insert(_root->children[comparator.Less(&_root->value, &_value)],
                          _value,
                          _root,
                          _created);


        _root->ColorAndCount.Count++;

        return;
    }
    



    template <typename...args_t>
    Node * Find(Node * iterator, args_t...arg)
    {
        while (iterator!=NULL)
        {
	    int compareResult = comparator.CompareSortFields(&iterator->value, arg...);
	    
            if (compareResult==0)
                return iterator;
            else
                iterator = iterator->children[!(compareResult < 0)];

        }

        return iterator;
    }
    
    template <typename...args_t>
    Node * FindWithIndex(Node * iterator, int& index, args_t...arg)
    {
        // If we have a child on the left, we will start with count of descendants on the left,
        // otherwise: we will start with count of 0
        if (iterator->children[LEFT]!=NULL)
            index = iterator->children[LEFT]->ColorAndCount.Count;
        else
            index = 0;
            
	    
        while (iterator!=NULL)
        {	    
	    int compareResult = comparator.CompareSortFields(&iterator->value, arg...);
	    
            if (compareResult==0)
	    {				
                return iterator;
	    }
            else
            {
                bool takeLeft;
		
		takeLeft=(compareResult > 0);
                
                iterator = iterator->children[compareResult < 0];
		
						
                
		if (iterator==NULL)
		{
			index = NotFound;
			
			return iterator;
		}
		
                if (takeLeft)
                {
                    index--;
                    
                    if (iterator->children[RIGHT]!=NULL)
                        index -= iterator->children[RIGHT]->ColorAndCount.Count;
                } else {
                    index++;
                    
                    if (iterator->children[LEFT]!=NULL)
                        index += iterator->children[LEFT]->ColorAndCount.Count;
                }
            }
        }
        
        if (iterator==NULL)
            index = NotFound;

        return iterator;
    }

public:
    class Iterator
    {
    protected:
        Node * node;

    public:
    	Iterator() : node(NULL)
	{
	}
	
        Iterator(Node* _node):
                node(_node)
        {
        }

        bool operator==(const Iterator& other) const
        {
            return node==other.node;
        }

        bool operator!=(const Iterator& other) const
        {
            return node!=other.node;
        }

        Value& operator*()
        {
            return node->value;
        }

        Value* operator->()
        {
            return &node->value;
        }

        Iterator operator++()
        {           
            Node * iterator = node->Successor();

            return Iterator(node = iterator);
        }


        Iterator operator++(int)
        {
            Node * oldNode = node;        
            Node * iterator = node->Successor();

            node = iterator;

            return Iterator(oldNode);
        }
        
        Iterator operator--()
        {
            Node * iterator = node->Predecessor();
            
            return Iterator(node = iterator);
        }
        
        Iterator operator--(int)
        {
            Node * oldNode = node;
            Node * iterator = node->Predecessor();
            
            node = iterator;
            
            return Iterator(oldNode);
        }
	
	Node* GetNode()
	{
		return node;
	}

        friend class SortableRandomAccessTree;
    };
    
private:
    void Sort(int low,int high, Iterator lowIterator, Iterator highIterator)
    {
	 int pivot, j, i;
	 Iterator pivotIterator, jIterator, iIterator;
	 
	 if (low<high)
 	 {
		pivot = low;
  		i = low;
  		j = high;
		
		iIterator = lowIterator;
		jIterator = highIterator;
		Value * pivot = &*jIterator;
 
	  	while(i<=j)
		{
   			//while((arr[i]<=arr[pivot])&&(i<high))
			while (comparator.Less(&*iIterator, pivot))
		   	{
			    i++;
			    iIterator++;
		        }
 
			//while(arr[j]>arr[pivot])
			while (comparator.Less(pivot, &*jIterator))
   			{
    			    j--;
			    jIterator--;
   			}
			
			if(i <= j)
   			{ 
				Node *iNode = iIterator.GetNode(), *jNode = jIterator.GetNode();
				
				if (root==iNode)
					root = jNode;
				else if (root==jNode)
					root = iNode;
				
				iNode->Swap(jNode);

				if (iIterator==lowIterator)
				{
					lowIterator = jIterator;
				}
				else if (jIterator==lowIterator)
				{
					lowIterator = iIterator;
				}
				
				if (iIterator==highIterator)
				{
					highIterator = jIterator;
				} else if (jIterator==highIterator)
				{
					highIterator = iIterator;
				}
									
				std::swap(iIterator, jIterator);
				
				
				i++;
				iIterator++;
    			        j--;
			        jIterator--;

				
   			}
  		}
 		
	
		if (low < j)		
	  		Sort(low,j, lowIterator, jIterator);
		
		if (i < high)
	  		Sort(i, high, iIterator, highIterator);
 	}
     }


public:
    static const int NotFound = -1;
public:
    SortableRandomAccessTree(const DynamicComparatorBuilder<Value> &_comparator) : root(NULL), leftMost(NULL), rightMost(NULL), comparator(_comparator)
    {
    	if (comparator.Empty())
		throw std::logic_error("SortableRandomAccessTree::ctor - Need at least one field to compare");
    }

    ~SortableRandomAccessTree()
    {
        if (root)
            delete root;
    }

    Iterator begin() const { return Iterator(leftMost); }
    Iterator end() const { return Iterator(NULL); }
    Iterator final() const { return Iterator(rightMost); }

    
    int size() const
    {
        if (root!=NULL)
            return root->ColorAndCount.Count;
        return 0;
    }


    template<typename...args_t>
    Iterator Find(args_t...args)
    {
        return Iterator(Find(root,args...));
    }
    
    template<typename...args_t>
    Iterator FindWithIndex(int& index,args_t...args)
    {
        return Iterator(FindWithIndex(root, index, args...));
    }

    Iterator GetByIndex(int index)
    {
        Node * iterator = root;
        int Counter;

        // If we have a child on the left, we will start with count of descendants on the left,
        // otherwise: we will start with count of 0
        if (iterator->children[LEFT]!=NULL)
            Counter = iterator->children[LEFT]->ColorAndCount.Count;
        else
            Counter = 0;

        while ((iterator!=NULL) && (Counter!=index))
        {
            // If the current going counter is less than the index we are looking for, then
            // we would descend into our descendants on the right.
            if (Counter < index)
            {
                // every time we descend on the right we would raise the counter.
                Counter++;
                                
                iterator = iterator->children[RIGHT];

                // Every time we descend to the descendant on the right, we would add to the counter all its descendants on the left.
                if (iterator->children[LEFT]!=NULL)
                    Counter += iterator->children[LEFT]->ColorAndCount.Count;
            } else {
                // otherwise: we will descend on the left
                
                // every time we descend on the left, we would decrease the counter.
                Counter--;

                iterator = iterator->children[LEFT];

                // Every time we descend on the left, we would decrease the counter by the count of all the descendants on the right.
                if (iterator->children[RIGHT]!=NULL)
                    Counter -= iterator->children[RIGHT]->ColorAndCount.Count;
            }
        }

    // When we exit the loop either we have iterator for the item at the rank, or end().
        return Iterator(iterator);
    }



    Iterator Delete(const Iterator& it)
    {
        Iterator nextNode(it);
        Node* iterator = it.node, * workingNode = iterator, * substitute = NULL,* substituteParent = NULL;

        // Record the next node in the tree to return to the caller.
        nextNode++;

        // Update all the ancestors to exclude this child.
        while (workingNode!=NULL)
        {
            workingNode->ColorAndCount.Count--;

            workingNode = workingNode->parent;
        }

        // We shall start with the node the user wishes to delete.
        workingNode = iterator;

        // If there is no child on the left, it means we may have one child:
        if (workingNode->children[LEFT] == NULL)
        {
            // we will take that child as the node to substitute the node being removed from the B-Tree.
            substitute = workingNode->children[RIGHT];
        } else
        {
            // If there is no child on the right, then we may have a child on the left:
            if (workingNode->children[RIGHT] == NULL)
            {
                // we will take that child as the node to substitute the node being removed from the B-Tree.
                substitute = workingNode->children[LEFT];
            } else {
                // otherwise: we will try to find the descendant with the lowest key greater than ours.
                workingNode = workingNode->children[RIGHT];

                while (workingNode->children[LEFT] != NULL)
                    workingNode = workingNode->children[LEFT];

                // workingNode will now contain the node that will replace the node being deleted.
                // substitute will contain its right child      
                substitute = workingNode->children[RIGHT];
            }
        }

        // If the node we are working on is not the one that was meant to be deleted:
        // (essentially this is the case where the node to be deleted had two children)
        if (workingNode != iterator)
        {
            /*
                    Cases: 
                    
                    Case 1:
                    
                    
                        I                          W 
                       / \                         /\
                      L  R(*)        becomes:     L R(*)
                        /                           /
                       W                           S
                        \
                         S
                    
                    * R may represents a sub-tree, where W is the minimum of the sub-tree, and S is 
                    the right child of the sub-tree.     
                         
                    Case 2:
                    
                        I                   W
                       /\                  /\
                      L W       becomes:  L  S  
                         \
                          S 
                          
                    
                    
             */
            // We will transfer the ownership of all the descendants on our left side to the lowest key
            // greater than ours.
            iterator->children[LEFT]->parent = workingNode;

            // We will record the Node count at this point to preserve our ability to search by rank, by
            // later on correctly adjusting the appropriate ancestors and/or descendants.
            int oldWorkingNodeCount = workingNode->ColorAndCount.Count;
            Node * itUpdateParent = workingNode->parent;
            int LeftChildrenCount = iterator->children[LEFT]->ColorAndCount.Count;

            // Since we are removing the "workingNode" from its portion of the tree, we need to
            // update all the parents to reflect this change.
            while (itUpdateParent!=iterator)
            {
                itUpdateParent->ColorAndCount.Count--;
                itUpdateParent = itUpdateParent->parent;
            }

            // Since the workingNode is adopting our descendants, we will give it the ownership of the count too.
            workingNode->ColorAndCount.Count += LeftChildrenCount;

            // make descendants on the left be the descendants of the working node.
            workingNode->children[LEFT] = iterator->children[LEFT];

            // if the workingNode is not the dying node's right child, then we can basically
            // move our right child to it's safely new minimum safely
            if (workingNode != iterator->children[RIGHT])
            {
                // Compute the count of the right children adjusted for the fact that we are taking away
                // the workingNode from the right tree.
                int RightChildrenCount = iterator->children[RIGHT]->ColorAndCount.Count - oldWorkingNodeCount + 1;

                workingNode->ColorAndCount.Count += RightChildrenCount;
                
                // Our former parent will adopt our child substitute.
                substituteParent = workingNode->parent;


                if (substitute)
                {
                    substitute->parent = workingNode->parent;
                }

                workingNode->parent->children[LEFT] = substitute;
                workingNode->children[RIGHT] = iterator->children[RIGHT];

                iterator->children[RIGHT]->parent = workingNode;
            } else {
                // otherwise: we are the parent of substitute
                substituteParent = workingNode;
            }

            // check to see if the root is changing, otherwise place our new node
            // where the node being deleted was.
            if (root == iterator)
                root = workingNode;
            else
                iterator->parent->children[!iterator->IsLeftChild()] = workingNode;

            // The deleted node's parent becomes our new replacement node's parent
            workingNode->parent = iterator->parent;

            // swap color
            color_t tColor = workingNode->ColorAndCount.Color;

            workingNode->ColorAndCount.Color = iterator->ColorAndCount.Color;
            iterator->ColorAndCount.Color = tColor;

            workingNode = iterator;
        } else {
            // otherwise: we have at least one null child, and therefore our
            // substitute's parent will be our former parent
            substituteParent = workingNode->parent;

            // if we have one non-null child then that child's parent is our former parent.
            if (substitute)
                substitute->parent = workingNode->parent;

            // if we were the root, then our child is the new root, otherwise: our child is our
            // parent's child instead of us.
            if (root == iterator)
                root = substitute;
            else
                iterator->parent->children[!iterator->IsLeftChild()] = substitute;

            // if we were the leftMost child, then we need a new leftMost child for the tree.
            if (leftMost == iterator)
            {
                if (iterator->children[RIGHT] == NULL)
                   leftMost = iterator->parent;
                else
                   leftMost = substitute->Minimum();
            }

            // if we were the rightMost child, then we need a new rightMost child for the tree.
            if (rightMost == iterator)
            {
                if (iterator->children[LEFT] == NULL)
                    rightMost = iterator->parent;
                else
                    rightMost = substitute->Maximum();
            }
        }

        // Standard rebalancing algorithm from Wikipedia, adjusted to have NULL represent null-leaves, instead
        // of having actual leaves in place. So all checks for BLACK also check for NULL being there instead,
        // and vice versa for RED. 
        if (workingNode->ColorAndCount.Color != RED)
        {
            // Wikipedia Case 1: only applies on a BLACK node,
            // and case 2 only applies when node is not root
            while (substitute != root && (substitute == NULL || substitute->ColorAndCount.Color == BLACK))
                // If we are a left child, we would move this
                // check that repeatedly occurs in Wikipedia
                // implementation to the top, so it is only done
                // once
                if (substituteParent->children[LEFT]==substitute)
                {
                    // Case 2 from Wikipedia
                    Node* sibling = substituteParent->children[RIGHT];

                    if (sibling->ColorAndCount.Color == RED)
                    {
                        sibling->ColorAndCount.Color = BLACK;
                        substituteParent->ColorAndCount.Color = RED;
                        
                        substituteParent->PivotLeft(root);
                        
                        sibling = substituteParent->children[RIGHT];
                    }
                    
                    // Case 3 from Wikipedia
                    if (((sibling->children[LEFT] == NULL) ||
                         (sibling->children[LEFT]->ColorAndCount.Color == BLACK)) &&
                        ((sibling->children[RIGHT] == NULL) ||
                         (sibling->children[RIGHT]->ColorAndCount.Color == BLACK)))
                    {
                        sibling->ColorAndCount.Color = RED;
                        substitute = substituteParent;
                        substituteParent = substituteParent->parent;
                        // By not breaking out of the loop, we 
                        // go back to case 1
                    } else {
                       // Case 4 from Wikipedia, combined with
                       // case 5
                        if ((sibling->children[RIGHT] == NULL) ||
                            (sibling->children[RIGHT]->ColorAndCount.Color == BLACK))
                        {
                            // This is from Case 5, eliminating shared if branches
                            if (sibling->children[LEFT])
                                sibling->children[LEFT]->ColorAndCount.Color = BLACK;

                            // Case 4
                            sibling->ColorAndCount.Color = RED;

                            sibling->PivotRight(root);
                            sibling = substituteParent->children[RIGHT];
                        }

                        // Remainder of Case 5
                        sibling->ColorAndCount.Color = substituteParent->ColorAndCount.Color;

                        substituteParent->ColorAndCount.Color = BLACK;

                        if (sibling->children[RIGHT])
                            sibling->children[RIGHT]->ColorAndCount.Color = BLACK;

                        substituteParent->PivotLeft(root);

                        break;
                }
            } else {
                // otherwise: We are right child, so our sibling is
                // on the left
                
                // Case 2 from Wikipedia
                Node* sibling = substituteParent->children[LEFT];
                
                if (sibling->ColorAndCount.Color == RED)
                {
                    sibling->ColorAndCount.Color = BLACK;
                    substituteParent->ColorAndCount.Color = RED;
                    substituteParent->PivotRight(root);
                    sibling = substituteParent->children[LEFT];
                }
                
                // Case 3 from Wikipedia
                if (((sibling->children[RIGHT] == NULL) ||
                     (sibling->children[RIGHT]->ColorAndCount.Color == BLACK)) &&
                    ((sibling->children[LEFT] == NULL) ||
                     (sibling->children[LEFT]->ColorAndCount.Color == BLACK)))
                {
                    sibling->ColorAndCount.Color = RED;
                    substitute = substituteParent;
                    substituteParent = substituteParent->parent;
                    
                    // By not breaking out of the loop, we 
                    // go back to case 1
                } else {
                    // Case 4 from Wikipedia, combined with
                    // case 5                
                    if ((sibling->children[LEFT] == NULL) ||
                        (sibling->children[LEFT]->ColorAndCount.Color == BLACK))
                    {
                    
                        // This is from Case 5, eliminating shared if branches                    
                        if (sibling->children[RIGHT])
                            sibling->children[RIGHT]->ColorAndCount.Color = BLACK;

                        // Case 4
                        sibling->ColorAndCount.Color = RED;
                        sibling->PivotLeft(root);
                        sibling = substituteParent->children[LEFT];
                    }
                    
                    // Remainder of Case 5                    
                    sibling->ColorAndCount.Color = substituteParent->ColorAndCount.Color;
                    substituteParent->ColorAndCount.Color = BLACK;

                    if (sibling->children[LEFT])
                        sibling->children[LEFT]->ColorAndCount.Color = BLACK;

                    substituteParent->PivotRight(root);
                    break;
                }
            }
            if (substitute)
                substitute->ColorAndCount.Color = BLACK;
        }


        workingNode = iterator;



        iterator->children[LEFT] = iterator->children[RIGHT] = NULL;

        delete iterator;

        return nextNode;
    }
    
    void Delete(const Iterator& first, const Iterator& last)
    {
        while (first!=last)
        {
            first = Delete(first);
        }
    }
    
    void Clear()
    {
        Delete(begin(), end());
    }
    
    void Resort(const DynamicComparatorBuilder<Value> &_comparator)
    {
    	if (!_comparator.Empty())
	    	comparator = _comparator;
	else
		throw std::logic_error("SortableRandomAccessTree::Resort - Need at least one field to compare");
	
	int count;
	
	if ((count = size()))
	{
		Sort(0, count-1, begin(), final());
		
		leftMost = root->Minimum();
		rightMost = root->Maximum();
	} 
    }

    Iterator Insert(const Value& value)
    {
        Node * iterator, *newNode;

        // Insert as we normally do into the BST tree
        Insert(root, value, NULL, newNode);

        // if the new insertion is the leftMost child then we must update our member containing
        // that node
        if (((leftMost!=NULL) && (leftMost->children[LEFT]==newNode)) || (leftMost==NULL))
            leftMost = newNode;

    // if the new insertion is the rightMost child then we must update our member containing
    // that node
        if (((rightMost!=NULL) && (rightMost->children[RIGHT]==newNode)) || (rightMost==NULL))
            rightMost = newNode;

        iterator = newNode;


        // Wikipedia - Red-Black Tree : Case 1
        while (true)
        {
            if (iterator->parent==NULL)
            {
                iterator->ColorAndCount.Color = BLACK;
                return Iterator(newNode);
            } else {
                // Wikipedia - Red-Black Tree : Case 2
                if (iterator->parent->ColorAndCount.Color==BLACK)
                    return Iterator(newNode);
                else {
                    // Wikipedia - Red-Black Tree : Case 3
                    Node * Uncle = iterator->GetUncle(), * GrandParent;

                    if ((Uncle!=NULL) && (Uncle->ColorAndCount.Color==RED))
                    {
                        iterator->parent->ColorAndCount.Color = BLACK;
                        Uncle->ColorAndCount.Color = BLACK;
                        GrandParent = iterator->GetGrandParent();
                        GrandParent->ColorAndCount.Color = RED;

                        iterator = GrandParent;

                        continue;
                    } else {
                        // Wikipedia Red-Black Tree : Case 4

                        if (iterator->IsRightChild() &&  iterator->parent->IsLeftChild())
                        {
                            iterator->parent->PivotLeft(root);
                            iterator = iterator->children[LEFT];
                        } else if (iterator->IsLeftChild() && iterator->parent->IsRightChild())
                        {
                            iterator->parent->PivotRight(root);
                            iterator = iterator->children[RIGHT];
                        }

                        GrandParent = iterator->GetGrandParent();

                        iterator->parent->ColorAndCount.Color = BLACK;
                        GrandParent->ColorAndCount.Color = RED;

                        if (iterator->IsLeftChild())
                            GrandParent->PivotRight(root);
                        else
                            GrandParent->PivotLeft(root);

                        return Iterator(newNode);

                    }
                }
            }
        }


    }
};
#endif
