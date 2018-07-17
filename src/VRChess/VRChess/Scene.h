#pragma once
#include "Model.h"
#include "Engine/Board.h"
#include "Engine/Search.h"
using namespace SjelkjdChessEngine;

enum GameState
{
	GameOver,
	PlayerMoving,
	PlayerPickingTargetSquare,
	PlayerPickingPromotionType,
	ComputerToMove,
	ComputerMoving,
	ComputerMoveComplete,
	GameStarting
};

class Scene
{	
private:
	unordered_map<int, unique_ptr<Model>> pieceModels;
	GameState gameState;
	thread t;
	int computerMove;
	int sourceRow, sourceCol;

	std::vector<unique_ptr<Model>> Models;
	unique_ptr<Model> sourceSquare, targetSquare;
	unique_ptr<Model> board;
	unique_ptr<Board> gameBoard, searchBoard;
	unique_ptr<Search> search;
	unique_ptr<MoveGenerator> moveGenerator;

	// runs on background thread
	void makeComputerMove();

	bool PickModel(XMFLOAT3& origin, XMFLOAT3& direction, XMMATRIX& viewMatrix, Model& model, float& tmin);
	void Init();
public:	
	Scene();

	void Render(XMMATRIX * proj, XMMATRIX * view);

	void RenderInstanced(XMMATRIX * projViews);
	
	Model* Pick(XMFLOAT3& origin, XMFLOAT3& direction, XMMATRIX& viewMatrix, float& tmin);
	
	bool GetBoardSquare(XMFLOAT3& origin, XMFLOAT3& direction, XMMATRIX& viewMatrix, int& row, int& col);
	
	void UpdatePointer(XMFLOAT3& origin, XMFLOAT3& direction, XMMATRIX& viewMatrix);
	void HandlePointerClick(XMFLOAT3& origin, XMFLOAT3& direction, XMMATRIX& viewMatrix);

	// call once per frame
	void Update();
};

