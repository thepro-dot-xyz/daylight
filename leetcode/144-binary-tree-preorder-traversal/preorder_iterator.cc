#include <vector>
#include <stack>
#include <iostream>

using namespace std;

struct TreeNode {
  int val;
  TreeNode *left;
  TreeNode *right;
  TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

class Solution {
public:
  vector<int> preorderTraversal(TreeNode *root)
  {
    stack<TreeNode *> tStack;
    vector<int> result;

    while (tStack.size() > 0 || root != NULL) {
      if (root != NULL) {
        result.push_back(root->val);
        if (root->right != NULL) {
          tStack.push(root->right);
        }
        root = root->left;
      } else {
        root = tStack.top();
        tStack.pop();
      }
    }
    return result;
  }
};

int main()
{
  TreeNode *root = new TreeNode(1);
  root->left = new TreeNode(2);
  root->right = new TreeNode(3);

  Solution s;
  vector<int> preorder = s.preorderTraversal(root);

  bool head = true;
  for (auto i : preorder) {
    if (!head) cout << ", ";
    cout << i;
    head = false;
  }
  cout << endl;

  return 0;
}
