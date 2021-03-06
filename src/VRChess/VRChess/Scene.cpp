#include "stdafx.h"
#include "Scene.h"
#include "MeshLoader.h"
#include "Engine/Types.h"

void Scene::Render(XMMATRIX * proj, XMMATRIX * view)
{
	for (auto i = Models.begin(); i != Models.end(); i++)
	{
		(*i)->Render(proj, view);
	}

	switch (gameState)
	{
	case GameOver:
		break;
	case PlayerMoving:
		sourceSquare->Render(proj, view);
		break;
	case PlayerPickingTargetSquare:
		sourceSquare->Render(proj, view);
		targetSquare->Render(proj, view);
		break;
	case PlayerPickingPromotionType:
		break;
	case ComputerMoving:
		break;
	}

	board->Render(proj, view);
	for (int i = 0; i < 64; i++)
	{
		int piece = gameBoard->GetPiece(i);
		if (piece != Pieces::None)
		{
			Model& model = *pieceModels[piece];
			model.Pos.x = GetRow(i) + 0.5f;
			model.Pos.z = GetCol(i) + 0.5f;
			model.Render(proj, view);
		}
	}
}

void Scene::Init()
{
	TriangleSet* walls = new TriangleSet();
	walls->AddSolidColorBox(10.1f, -0.1f, 20.0f, 10.0f, 8.0f, -20.0f, 0xff808080);  // Left Wall
	walls->AddSolidColorBox(10.0f, -0.1f, -20.0f, -10.0f, 8.0f, -20.0f, 0xff808080);  // Front Wall
	walls->AddSolidColorBox(10.0f, -0.1f, 20.0f, -10.0f, 8.0f, 20.0f, 0xff808080); // Back Wall
	walls->AddSolidColorBox(-10.0f, -0.1f, 20.0f, -10.1f, 8.0f, -20.0f, 0xff808080);   // Right Wall
	Models.push_back(unique_ptr<Model>(
		new Model(walls, XMFLOAT3(0, 0, 0), XMFLOAT4(0, 0, 0, 1),
			shared_ptr<Material>(new Material(
				new GeneratedTexture(false, 256, 256, GeneratedTexture::AUTO_WALL)
			))
		)
	));

	TriangleSet* floors = new TriangleSet();
	floors->AddSolidColorBox(10.0f, -0.1f, 20.0f, -10.0f, -0.01f, -20.1f, 0xff808080); // Main floor
	floors->AddSolidColorBox(15.0f, -6.1f, -18.0f, -15.0f, -6.0f, -30.0f, 0xff808080); // Bottom floor
	Models.push_back(unique_ptr<Model>(
		new Model(floors, XMFLOAT3(0, 0, 0), XMFLOAT4(0, 0, 0, 1),
			shared_ptr<Material>(new Material(
				new GeneratedTexture(false, 256, 256, GeneratedTexture::AUTO_FLOOR)
			))
		)
	)); // Floors

	TriangleSet* ceiling = new TriangleSet();
	ceiling->AddSolidColorBox(10.0f, 8.0f, 20.0f, -10.0f, 8.1f, -20.1f, 0xff808080);
	Models.push_back(unique_ptr<Model>(
		new Model(ceiling, XMFLOAT3(0, 0, 0), XMFLOAT4(0, 0, 0, 1),
			shared_ptr<Material>(new Material(
				new GeneratedTexture(false, 256, 256, GeneratedTexture::AUTO_CEILING)
			))
		)
	)); // Ceiling
	
	TriangleSet* sourceSquareTriangles = new TriangleSet();
	sourceSquareTriangles->AddSolidColorBox(0.45f, -0.05f, 0.45f, -0.45f, 0.05f, -0.45f, 0xff404040);
	sourceSquare.reset(new Model(sourceSquareTriangles, XMFLOAT3(0, 0, 0), XMFLOAT4(0, 0, 0, 1),
		shared_ptr<Material>(new Material(
			new GeneratedTexture(false, 256, 256, GeneratedTexture::AUTO_WHITE)
		))));

	TriangleSet* targetSquareTriangles = new TriangleSet();
	targetSquareTriangles->AddSolidColorBox(0.45f, -0.05f, 0.45f, -0.45f, 0.05f, -0.45f, 0xff404040);
	targetSquare.reset(new Model(targetSquareTriangles, XMFLOAT3(0, 0, 0), XMFLOAT4(0, 0, 0, 1),
		shared_ptr<Material>(new Material(
			new GeneratedTexture(false, 256, 256, GeneratedTexture::AUTO_WHITE)
		))));

	MeshLoader loader;
	
	pieceModels[Pieces::BlackBishop].reset(loader.LoadMeshFromFile("BBishop.txt"));
	pieceModels[Pieces::BlackKnight].reset(loader.LoadMeshFromFile("BKnight.txt"));
	pieceModels[Pieces::BlackKing].reset(loader.LoadMeshFromFile("BKing.txt"));
	pieceModels[Pieces::BlackQueen].reset(loader.LoadMeshFromFile("BQueen.txt"));
	pieceModels[Pieces::BlackPawn].reset(loader.LoadMeshFromFile("BPawn.txt"));
	pieceModels[Pieces::BlackRook].reset(loader.LoadMeshFromFile("BRook.txt"));
	pieceModels[Pieces::WhiteBishop].reset(loader.LoadMeshFromFile("WBishop.txt"));
	pieceModels[Pieces::WhiteKnight].reset(loader.LoadMeshFromFile("WKnight.txt"));
	pieceModels[Pieces::WhiteRook].reset(loader.LoadMeshFromFile("WRook.txt"));
	pieceModels[Pieces::WhiteKing].reset(loader.LoadMeshFromFile("WKing.txt"));
	pieceModels[Pieces::WhiteQueen].reset(loader.LoadMeshFromFile("WQueen.txt"));
	pieceModels[Pieces::WhitePawn].reset(loader.LoadMeshFromFile("WPawn.txt"));
	
	board.reset(loader.LoadMeshFromFile("Board.txt"));
	board->Pos = XMFLOAT3(0, 0, 0);
}

void Scene::UpdatePointer(XMFLOAT3& origin, XMFLOAT3& direction, XMMATRIX& viewMatrix)
{
	int row, col;
	switch (gameState)
	{
	case PlayerMoving:

		if (GetBoardSquare(origin, direction, viewMatrix, row, col))
		{
			sourceSquare->Pos.x = row + 0.5f;
			sourceSquare->Pos.z = col + 0.5f;
		}
		break;
	case PlayerPickingTargetSquare:

		if (GetBoardSquare(origin, direction, viewMatrix, row, col))
		{
			targetSquare->Pos.x = row + 0.5f;
			targetSquare->Pos.z = col + 0.5f;
		}
		break;
	}
}

void Scene::HandlePointerClick(XMFLOAT3& origin, XMFLOAT3& direction, XMMATRIX& viewMatrix)
{
	int row, col;
	switch (gameState)
	{
	case PlayerMoving:

		if (GetBoardSquare(origin, direction, viewMatrix, row, col))
		{
			vector<int> moves;
			moveGenerator->GenerateAllMoves(moves);
			bool hasMatchingMove = false;
			for (int move : moves)
			{
				int source = GetSourceFromMove(move);
				int moveRow = GetRow(source);
				int moveCol = GetCol(source);
				if (moveRow == row && moveCol == col)
				{
					hasMatchingMove = true;
					break;
				}
			}
			if (hasMatchingMove)
			{
				sourceRow = row;
				sourceCol = col;
				sourceSquare->Pos.x = row + 0.5f;
				sourceSquare->Pos.z = col + 0.5f;
				gameState = PlayerPickingTargetSquare;
			}
			
		}
		break;

	case PlayerPickingTargetSquare:

		if (GetBoardSquare(origin, direction, viewMatrix, row, col))
		{
			vector<int> moves;
			moveGenerator->GenerateAllMoves(moves);
			bool hasMatchingMove = false;
			int playerMove;
			for (int move : moves)
			{
				int source = GetSourceFromMove(move);
				int moveSourceRow = GetRow(source);
				int moveSourceCol = GetCol(source);

				int target = GetDestFromMove(move);
				int moveTargetRow = GetRow(target);
				int moveTargetCol = GetCol(target);
				if (moveSourceRow == sourceRow && moveSourceCol == sourceCol && moveTargetRow == row && moveTargetCol == col)
				{
					hasMatchingMove = true;
					playerMove = move;
					break;
				}
			}

			if (hasMatchingMove)
			{
				gameBoard->MakeMove(playerMove, true);
				searchBoard->MakeMove(playerMove, true);
				moves.clear();
				moveGenerator->GenerateAllMoves(moves);
				if (moves.size() > 0)
				{
					gameState = ComputerToMove;
				}
				else
				{
					gameState = GameOver;
				}

			}
			else
			{
				gameState = PlayerMoving;
			}
		}
		else
		{
			gameState = PlayerMoving;
		}
		break;
	}
}

void Scene::makeComputerMove()
{
	int move = search->GetBestMove(2.0);
	computerMove = move;
	gameState = ComputerMoveComplete;
}

void Scene::Update()
{
	switch (gameState)
	{
	case PlayerMoving:
		break;
	case PlayerPickingTargetSquare:
		break;
	case PlayerPickingPromotionType:
		break;
	case ComputerToMove:
		t = thread(&Scene::makeComputerMove, this);
		gameState = ComputerMoving;
		break;
	case ComputerMoving:
		break;
	case ComputerMoveComplete:
		t.join();
		gameBoard->MakeMove(computerMove, true);
		searchBoard->MakeMove(computerMove, true);
		vector<int> moves;
		moveGenerator->GenerateAllMoves(moves);
		if (moves.size() > 0)
		{
			gameState = PlayerMoving;
		}
		else
		{
			gameState = GameOver;
		}
		break;
	}
}

bool Scene::GetBoardSquare(XMFLOAT3& origin, XMFLOAT3& direction, XMMATRIX& viewMatrix, int& row, int& col)
{
	XMVECTOR viewMatrixDeterminant = XMMatrixDeterminant(viewMatrix);
	// Tranform ray to local space of Mesh.
	XMMATRIX invView = XMMatrixInverse(&viewMatrixDeterminant, viewMatrix);

	XMMATRIX W = XMMatrixMultiply(XMMatrixRotationQuaternion(XMLoadFloat4(&board->Rot)), XMMatrixTranslationFromVector(XMLoadFloat3(&board->Pos)));

	XMVECTOR wMatrixDeterminant = XMMatrixDeterminant(W);
	XMMATRIX invWorld = XMMatrixInverse(&wMatrixDeterminant, W);

	//ray already in local space
	XMMATRIX toLocal = invWorld;

	XMVECTOR transformedRayOrigin = XMVector3TransformCoord(XMLoadFloat3(&origin), toLocal);
	XMVECTOR transformedRayDir = XMVector3TransformNormal(XMLoadFloat3(&direction), toLocal);

	// Make the ray direction unit length for the intersection tests.
	transformedRayDir = XMVector3Normalize(transformedRayDir);

	float tmin = 0.0f;
	if (board->Pick(transformedRayOrigin, transformedRayDir, tmin))
	{
		XMVECTOR intersection = transformedRayOrigin + transformedRayDir * tmin;
		XMFLOAT3 intersectionxyz;
		XMStoreFloat3(&intersectionxyz, intersection);
		float x = intersectionxyz.x;
		float z = intersectionxyz.z;
		float u = x / 8.0f;
		float v = z / 8.0f;
		row = int(x);
		col = int(z);
		if (row >= 0 && row < 8 && col >= 0 && col < 8)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

Model* Scene::Pick(XMFLOAT3& origin, XMFLOAT3& direction, XMMATRIX& viewMatrix, float& tmin)
{
	tmin = 0.0f;
	Model* hit = nullptr;
	if (PickModel(origin, direction, viewMatrix, *board, tmin))
	{
		hit = board.get();
	}
	for (auto i = Models.begin(); i != Models.end(); i++)
	{
		if (PickModel(origin, direction, viewMatrix, **i, tmin))
		{
			hit = i->get();
		}
	}

	return hit;
}

bool Scene::PickModel(XMFLOAT3& origin, XMFLOAT3& direction, XMMATRIX& viewMatrix, Model& model, float& tmin)
{
	XMMATRIX W = XMMatrixMultiply(XMMatrixRotationQuaternion(XMLoadFloat4(&model.Rot)), XMMatrixTranslationFromVector(XMLoadFloat3(&model.Pos)));

	XMVECTOR wDeterminant = XMMatrixDeterminant(W);
	XMMATRIX invWorld = XMMatrixInverse(&wDeterminant, W);

	//ray already in local space
	XMVECTOR transformedRayOrigin = XMVector3TransformCoord(XMLoadFloat3(&origin), invWorld);
	XMVECTOR transformedRayDir = XMVector3TransformNormal(XMLoadFloat3(&direction), invWorld);

	// Make the ray direction unit length for the intersection tests.
	transformedRayDir = XMVector3Normalize(transformedRayDir);

	if (model.Pick(transformedRayOrigin, transformedRayDir, tmin))
	{
		return true;
	}
	return false;
}

Scene::Scene()
{
	gameState = PlayerMoving;
	gameBoard.reset(new Board());
	searchBoard.reset(new Board());
	search.reset(new Search(*searchBoard));
	moveGenerator.reset(new MoveGenerator(*gameBoard));
	gameBoard->SetFEN(Board::startPosition);	
	searchBoard->SetFEN(Board::startPosition);
	Init();
}

Scene::~Scene()
{
	// stop the search
	if (gameState == ComputerMoving || gameState == ComputerToMove)
	{
		search->SetTimeExceeded();
		t.join();
	}
}