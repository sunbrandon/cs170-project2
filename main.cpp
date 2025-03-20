#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <chrono>
#include <vector>

using namespace std;

bool loadData(const string &filename, vector<int>& labels, vector<vector<double> >& dataArray, int &numInstances, int &numFeatures) {
    ifstream fin(filename);
    if (!fin) {
        cout << "Error opening file: " << filename << endl;
        return false;
    }
    labels.clear();     // clear previous labels
    dataArray.clear();      // clear previous data
    string line;

    // while (fin >> value) {
    //     labels.push_back(static_cast<int>(value));
    //     vector<double> row;
    //     // Incorrectly assumed there are 10 features per row.
    //     for (int i = 0; i < 10; i++) {
    //         row.push_back(value);
    //     }
    //     dataArray.push_back(row);
    // }

    while (getline(fin, line)) {         // read file line by line
        if (line.empty()) {     // skip empty lines
            continue;
        }
        istringstream iss(line);
        double value;
        if (!(iss >> value)) {
            break;
        }
        labels.push_back(static_cast<int>(value));      // save the class label
        vector<double> row;

        // for (int col = 0; col < numFeatures; col++) {
        //     iss >> value;
        //     row.push_back(value);
        // }

        while (iss >> value) {
            row.push_back(value);
        }
        dataArray.push_back(row);
    }
    fin.close();
    if (!dataArray.empty()) {
        numInstances = dataArray.size();
        numFeatures = dataArray[0].size();
    }
    return true;
}

double euclideanDistance(int i, int j, const vector<int>& featureMask, const vector<vector<double> >& dataArray, int numFeatures) {
    double sum = 0.0;
    for (int f = 0; f < numFeatures; f++) {      // l   oop over each feature
        if (featureMask[f] == 1) {
            // sum += abs(dataArray[i][f] - dataArray[j][f]);

            double diff = dataArray[i][f] - dataArray[j][f];        // compute difference between instances i and j

            // sum += pow(diff, 2);
            sum += diff * diff;     // square the difference and add to sum
        }
    }
    return sqrt(sum);       // return the square root of the sum (Euclidean distance)
}

double leaveOneOutAccuracy(const vector<int>& featureMask, const vector<vector<double> >& dataArray, const vector<int>& labels, int numInstances, int numFeatures) {
    int correctCount = 0;
    for (int i = 0; i < numInstances; i++) {
        vector<int> indices;
        for (int j = 0; j < numInstances; j++) {
            if (j != i)
                indices.push_back(j);
        }

        // double bestDistance = 1e9;
        // int nearestLabel = -1;
        // for (int j = 0; j < numInstances; j++) {
        //     if (j == i) {
        //         continue
        //     };
        //     double dist = euclideanDistance(i, j, featureMask, dataArray, numFeatures);
        //     if (dist < bestDistance) {
        //         bestDistance = dist;
        //         nearestLabel = labels[j];
        //     }
        // }
        // if (nearestLabel == labels[i]) {
        //     correctCount++;
        // }

        // double avgDistance = 0.0;
        // for (int j = 0; j < numInstances; j++) {
        //     if (j != i)
        //         avgDistance += euclideanDistance(i, j, featureMask, dataArray, numFeatures);
        // }
        // avgDistance /= (numInstances - 1);
        // if (avgDistance < 5.0) { 
        //     correctCount++;
        // }

        // quicksort to sort 'indices' by distance from instance i
        vector<int> stack;
        stack.push_back(0);                     
        stack.push_back(indices.size() - 1);
        
        while (!stack.empty()) {
            int right = stack.back(); stack.pop_back();
            int left = stack.back(); stack.pop_back();
            if (left >= right) {
                continue;
            }
            int pivotIndex = right;
            double pivotDist = euclideanDistance(i, indices[pivotIndex], featureMask, dataArray, numFeatures);
            int storeIndex = left;
            for (int j = left; j < right; j++) {
                double currDist = euclideanDistance(i, indices[j], featureMask, dataArray, numFeatures);
                if (currDist < pivotDist) {
                    int temp = indices[j];
                    indices[j] = indices[storeIndex];
                    indices[storeIndex] = temp;
                    storeIndex++;
                }
            }
            int temp = indices[storeIndex];
            indices[storeIndex] = indices[right];
            indices[right] = temp;
            
            if (storeIndex - 1 > left) {
                stack.push_back(left);
                stack.push_back(storeIndex - 1);
            }
            if (storeIndex + 1 < right) {
                stack.push_back(storeIndex + 1);
                stack.push_back(right);
            }
        }
        
        int nearestLabel = labels[indices[0]];
        if (nearestLabel == labels[i])
            correctCount++;
    }
    return 100.0 * correctCount / numInstances;     // return the accuracy percentage
}



void printFeatureSet(const vector<int>& featureMask, int numFeatures) {
    cout << "{ ";
    for (int f = 0; f < numFeatures; f++) {
        if (featureMask[f] == 1) {
            cout << (f + 1) << " ";
        }
    }
    cout << "}";
}

double defaultRate(const vector<int>& labels) {
    if (labels.empty()) {
        return 0.0;
    }
    vector<int> sortedLabels = labels;       // copy labels to sort

    // int sum = 0;
    // for (int i = 0; i < labels.size(); i++) {
    //     sum += labels[i];
    // }
    // int mode = sum / labels.size();
    
    // quicksort to sort the labels
    vector<int> stack;
    stack.push_back(0);
    stack.push_back(sortedLabels.size() - 1);
    
    while (!stack.empty()) {
        int right = stack.back(); stack.pop_back();
        int left = stack.back(); stack.pop_back();
        
        if (left >= right)
            continue;
        
        int pivot = sortedLabels[right];
        int i = left - 1;
        for (int j = left; j < right; j++) {
            if (sortedLabels[j] <= pivot) {
                i++;
                int temp = sortedLabels[i];
                sortedLabels[i] = sortedLabels[j];
                sortedLabels[j] = temp;
            }
        }
        int pivotIndex = i + 1;
        int temp = sortedLabels[pivotIndex];
        sortedLabels[pivotIndex] = sortedLabels[right];
        sortedLabels[right] = temp;
        
        if (pivotIndex - 1 > left) {
            stack.push_back(left);
            stack.push_back(pivotIndex - 1);
        }
        if (pivotIndex + 1 < right) {
            stack.push_back(pivotIndex + 1);
            stack.push_back(right);
        }
    }
    
    int maxCount = 1;
    int currentCount = 1;
    for (size_t i = 1; i < sortedLabels.size(); i++) {
        if (sortedLabels[i] == sortedLabels[i - 1])
            currentCount++;
        else {
            if (currentCount > maxCount)
                maxCount = currentCount;
            currentCount = 1;
        }
    }
    if (currentCount > maxCount)
        maxCount = currentCount;
    
    return 100.0 * maxCount / sortedLabels.size();      // return default rate (empty set) as percentage
}


void forwardSelection(const vector<vector<double> >& dataArray, const vector<int>& labels, int numInstances, int numFeatures) {
    vector<int> currentMask(numFeatures, 0);
    vector<int> bestOverallMask(numFeatures, 0);    // vector to store the best overall feature set found
    double bestOverallAccuracy = 0.0;

    // bestOverallAccuracy = leaveOneOutAccuracy(currentMask, dataArray, labels, numInstances, numFeatures);
    
    double defRate = defaultRate(labels);
    cout << "Empty set (default rate) is " << fixed << setprecision(1) << defRate << "%" << endl << endl;
    
    cout << "Beginning search." << endl << endl;
    for (int level = 0; level < numFeatures; level++) {     // loop through for each level (number of features to add)
        int featureToAdd = -1;
        double bestAccuracyThisLevel = 0.0;
        vector<int> bestMaskThisLevel = currentMask;
        for (int f = 0; f < numFeatures; f++) {
            if (currentMask[f] == 0) {      // if feature f not selected
                vector<int> tempMask = currentMask;
                tempMask[f] = 1;        // temp add feature f
                double accuracy = leaveOneOutAccuracy(tempMask, dataArray, labels, numInstances, numFeatures);
                cout << "   Using feature(s) ";
                printFeatureSet(tempMask, numFeatures);
                cout << " accuracy is " << fixed << setprecision(1) << accuracy << "%" << endl;
                if (accuracy > bestAccuracyThisLevel) {     // update best accuracy for this level
                    bestAccuracyThisLevel = accuracy;
                    featureToAdd = f;
                    bestMaskThisLevel = tempMask;
                }
            }
        }
        if (featureToAdd != -1) {       
            currentMask[featureToAdd] = 1;
            cout << endl << "Feature set ";
            printFeatureSet(currentMask, numFeatures);
            cout << " was best, accuracy is " << fixed << setprecision(1) << bestAccuracyThisLevel << "%" << endl;
            if (bestAccuracyThisLevel < bestOverallAccuracy) {
                cout << endl << "(WARNING, Accuracy has decreased! Continuing search in case of local maxima)" << endl;
            }
            if (bestAccuracyThisLevel > bestOverallAccuracy) {      // update overall best if improved
                bestOverallAccuracy = bestAccuracyThisLevel;
                bestOverallMask = currentMask;
            }
            cout << endl;
        }
    }
    cout << "Finished search!! The best feature subset is ";
    printFeatureSet(bestOverallMask, numFeatures);
    cout << ", which has an accuracy of " << fixed << setprecision(1) << bestOverallAccuracy << "%" << endl;
}

void backwardElimination(const vector<vector<double> >& dataArray, const vector<int>& labels, int numInstances, int numFeatures) {
    vector<int> currentMask(numFeatures, 1);
    vector<int> bestOverallMask(numFeatures, 1);
    double bestOverallAccuracy = leaveOneOutAccuracy(currentMask, dataArray, labels, numInstances, numFeatures);
    cout << "Beginning search." << endl << endl;
    cout << "   Using feature(s) ";
    printFeatureSet(currentMask, numFeatures);
    cout << " accuracy is " << fixed << setprecision(1) << bestOverallAccuracy << "%" << endl << endl;
    
    for (int level = 0; level < numFeatures - 1; level++) {     // iteratively remove one feature at a time
        int featureToRemove = -1;
        double bestAccuracyThisLevel = 0.0;
        vector<int> bestMaskThisLevel = currentMask;
        for (int f = 0; f < numFeatures; f++) {
            if (currentMask[f] == 1) {      // feature is currently selected
                vector<int> tempMask = currentMask;
                tempMask[f] = 0;        // remove feature f temporarily
                double accuracy = leaveOneOutAccuracy(tempMask, dataArray, labels, numInstances, numFeatures);
                cout << "   Removing feature " << (f + 1) << " from ";
                printFeatureSet(currentMask, numFeatures);
                cout << " yields accuracy of " << fixed << setprecision(1) << accuracy << "%" << endl;
                if (accuracy > bestAccuracyThisLevel) {
                    bestAccuracyThisLevel = accuracy;
                    featureToRemove = f;
                    bestMaskThisLevel = tempMask;
                }
            }
        }
        if (featureToRemove != -1) {        // if removing a feature improves accuracy
            currentMask[featureToRemove] = 0;       // remove
            cout << endl << "Feature set ";
            printFeatureSet(currentMask, numFeatures);
            cout << " was best, accuracy is " << fixed << setprecision(1) << bestAccuracyThisLevel << "%" << endl;
            if (bestAccuracyThisLevel < bestOverallAccuracy) {
                cout << endl << "(WARNING, Accuracy has decreased! Continuing search in case of local maxima)" << endl;
            }
            if (bestAccuracyThisLevel > bestOverallAccuracy) {
                bestOverallAccuracy = bestAccuracyThisLevel;
                bestOverallMask = currentMask;
            }
            cout << endl;
        }
    }

    double defRate = defaultRate(labels);
    cout << "Empty set (default rate) is " << fixed << setprecision(1) << defRate << "%" << endl << endl;

    cout << "Finished search!! The best feature subset is ";
    printFeatureSet(bestOverallMask, numFeatures);
    cout << ", which has an accuracy of " << fixed << setprecision(1) << bestOverallAccuracy << "%" << endl;
}

int main() {
    auto start = chrono::steady_clock::now();       // record start time
    
    cout << "Welcome to Brandon Sun's Feature Selection Algorithm." << endl << endl;
    cout << "Type in the name of the file to test: ";
    string filename;
    cin >> filename;
    cout << endl;
    
    cout << "Type the number of the algorithm you want to run." << endl;
    cout << "   1) Forward Selection" << endl;
    cout << "   2) Backward Elimination" << endl;
    int choice;
    cin >> choice;
    cout << endl;
    
    vector<int> labels;
    vector<vector<double> > dataArray;      // 2D vector to hold features
    int numInstances = 0;
    int numFeatures = 0;
    
    if (!loadData(filename, labels, dataArray, numInstances, numFeatures)) {
        return 1;
    }
    
    cout << "This dataset has " << numFeatures << " features (not including the class attribute), with " << numInstances << " instances." << endl << endl;
    
    vector<int> allFeatures(numFeatures, 1);
    double allAccuracy = leaveOneOutAccuracy(allFeatures, dataArray, labels, numInstances, numFeatures);
    cout << "Running nearest neighbor with all " << numFeatures << " features, using \"leave-one-out\" evaluation, I get an accuracy of " << fixed << setprecision(1) << allAccuracy << "%" << endl << endl;
    
    if (choice == 1) {
        forwardSelection(dataArray, labels, numInstances, numFeatures);
    }
    else if (choice == 2) {
        backwardElimination(dataArray, labels, numInstances, numFeatures);
    }
    else {
        cout << "Invalid choice. Please run again and select 1 or 2." << endl;
    }
    
    auto end = chrono::steady_clock::now();     // record end time
    chrono::duration<double> elapsed_seconds = end - start;     // compute elapsed time
    cout << "Time taken: " << elapsed_seconds.count() << " seconds" << endl;
    
    return 0;
}
