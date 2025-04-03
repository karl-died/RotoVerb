//
//  GivensRotation.h
//  PackedGivensMatrix
//
//  Created by Karl Diedrichsen on 01.04.25.
//
#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <array>
#include <iostream>

//constexpr int numAnglesForOrder(int n) { return ((n - 1) * n) / 2; }
//constexpr int numPackedMatricesForOrder(int n) { return n - 1; }

template <int N>
class GivensRotation
{
public:
    static constexpr int numAngles = ((N - 1) * N) / 2;
    static constexpr int numPackedMatrices = N - 1;
    
    
    
    GivensRotation()
    {
        std::array<std::array<int, N>, numPackedMatrices> packedIndeces{};
        std::array<std::array<bool, N>, N> usedMatrices{};
        
        for(int i = 0; i < numPackedMatrices; i++)
            packedIndeces[i][1] = i + 1;
        
        long int numIterations = 0;
        std::function<bool(int, int, int, int)> fill = [&] (int i, int j, int k, int l)
        {
            numIterations++;
            if(j > (N / 2) - 1)
            {
                j = 1;
                i++;
                k = 1;
                l = 2;
                if(i >= numPackedMatrices)
                    return true;
            }
            
            while(true)
            {
                if(l >= N)
                {
                    k++;
                    l = k + 1;
                }
                
                if(k >= N - 1)
                    return false;
                
                if(usedMatrices[k][l] == false)
                    break;
                
                l++;
            }
            
            for(int a : packedIndeces[i])
                if(a == k)
                    return fill(i, j, k + 1, k + 2);
               
            for(int a : packedIndeces[i])
                if(a == l)
                    return fill(i, j, k, l + 1);

            
            packedIndeces[i][2 * j]     = k;
            packedIndeces[i][2 * j + 1] = l;
            
            usedMatrices[k][l] = true;
            
            bool result = fill(i, j + 1, k + 1, k + 2);
            
            if(result == false)
            {
                packedIndeces[i][2 * j]     = 0;
                packedIndeces[i][2 * j + 1] = 0;
                
                usedMatrices[k][l] = false;
                return fill(i, j, k, l + 1);
            }
            else
            {
                return true;
            }
        };
        
        bool result = fill(0, 1, 1, 2);
        
        std::array<std::array<int, N>, N> angleIndeces{};
        
        int counter = 0;
        
        for(int i = 0; i < N + 1; i++)
            for(int j = i + 1; j < N; j++)
                angleIndeces[i][j] = counter++;
        
        for(int i = 0; i < numPackedMatrices; i++)
        {
            for(int j = 0; j < N / 2; j++)
            {
                int x = packedIndeces[i][2 * j];
                int y  = packedIndeces[i][2 * j + 1];
                packedMatrixMapping[i][j].x = x;
                packedMatrixMapping[i][j].y = y;
                packedMatrixMapping[i][j].angleIndex =  angleIndeces[x][y];
            }
        }
        std::cout << result << std::endl;
    }
    
    Eigen::Matrix<float, 1, N> rotate(const std::array<float, numAngles>& angles, const Eigen::Matrix<float, N, 1>& input)
    {
        static std::array<Eigen::Matrix<float, N, N>, numPackedMatrices> packedMatrices{};
        
        for(int i = 0; i < numPackedMatrices; i++)
        {
            for(int j = 0; j < N / 2; j++)
            {
                MatrixEntry me = packedMatrixMapping[i][j];
                packedMatrices[i](me.x, me.x) =  cos(angles[me.angleIndex]);
                packedMatrices[i](me.x, me.y) =  sin(angles[me.angleIndex]);
                packedMatrices[i](me.y, me.x) = -sin(angles[me.angleIndex]);
                packedMatrices[i](me.y, me.y) =  cos(angles[me.angleIndex]);
            }
        }
        
        Eigen::Matrix<float, N, 1> result{input};
        
        for(int i = 0; i < numPackedMatrices; i++)
            result = (result.transpose() * packedMatrices[i]);
        
        return result;
    }
    
    Eigen::Matrix<float, 1, N> rotateSparse(const std::array<float, numAngles>& angles, const Eigen::Matrix<float, N, 1>& input)
    {
        std::array<Eigen::SparseMatrix<float>, numPackedMatrices> packedMatrices{};
        
        for(int i = 0; i < numPackedMatrices; i++)
        {
            packedMatrices[i] = Eigen::SparseMatrix<float>(N, N);
            std::vector<Eigen::Triplet<float>> triplets{};
            
            for(int j = 0; j < N / 2; j++)
            {
                MatrixEntry me = packedMatrixMapping[i][j];
                
                triplets.emplace_back(me.x, me.y,  cos(angles[me.angleIndex]));
                triplets.emplace_back(me.x, me.y,  sin(angles[me.angleIndex]));
                triplets.emplace_back(me.x, me.y, -sin(angles[me.angleIndex]));
                triplets.emplace_back(me.x, me.y,  cos(angles[me.angleIndex]));
            }
            
            packedMatrices[i].setFromTriplets(triplets.begin(), triplets.end());
        }
        
        Eigen::Matrix<float, N, 1> result{input};
        
        for(int i = 0; i < numPackedMatrices; i++)
            result = (result.transpose() * packedMatrices[i]);
        
        return result;
    }
    
private:
    
    struct MatrixEntry
    {
        short int x;
        short int y;
        short int angleIndex;
    };
    
    std::array<std::array<MatrixEntry, N / 2>, numPackedMatrices> packedMatrixMapping{};
};

