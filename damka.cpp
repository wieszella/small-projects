#include <iostream>
#include <vector>
#include <string>
#include <cctype>

const int BOARD_SIZE = 8;

enum Player {
    PLAYER_1,
    PLAYER_2
};

enum Piece {
    EMPTY,
    PLAYER_1_PIECE,
    PLAYER_1_KING,
    PLAYER_2_PIECE,
    PLAYER_2_KING
};

struct Move {
    int fromRow, fromCol, toRow, toCol;
};

class CheckersGame {
private:
    std::vector<std::vector<Piece>> board;
    Player currentPlayer;

public:
    CheckersGame() : board(BOARD_SIZE, std::vector<Piece>(BOARD_SIZE, EMPTY)), currentPlayer(PLAYER_1) 
    {
        //initializeBoard();
        initializeBoardDebug();
    }

    void play() 
    {
        while (!isGameOver()) 
        {
            printBoard();
            std::cout << "Player " << currentPlayer + 1 << "'s turn\n";
            if (makeMove()) 
            {
                switchPlayer();
            }
            system("cls");
        }

        printBoard();
        std::cout << "Player " << getWinner() + 1 << " wins!\n";
    }

private:
    void initializeBoard() 
    {
        for (int row = 0; row < BOARD_SIZE; ++row) 
        {
            for (int col = 0; col < BOARD_SIZE; ++col) 
            {
                if ((row + col) % 2 != 0) 
                {
                    if (row < 3) 
                    {
                        board[row][col] = PLAYER_1_PIECE;
                    }
                    else if (row > BOARD_SIZE - 4)
                    {
                        board[row][col] = PLAYER_2_PIECE;
                    }
                    else {
                        board[row][col] = EMPTY;
                    }
                }
            }
        }
    }

    void initializeBoardDebug()
    {
        for (int row = 0; row < BOARD_SIZE; ++row)
        {
            for (int col = 0; col < BOARD_SIZE; ++col)
            {
                board[row][col] = EMPTY;
            }
        }
        board[0][7] = PLAYER_1_KING;
        board[4][3] = PLAYER_2_KING;
        board[5][2] = PLAYER_2_KING;
    }

    void printBoard() 
    {
        std::cout << "  ";
        for (int i = 0; i < BOARD_SIZE; ++i) 
        {
            std::cout << i << " ";
        }
        std::cout << "\n";

        for (int row = 0; row < BOARD_SIZE; ++row) 
        {
            std::cout << row << " ";
            for (int col = 0; col < BOARD_SIZE; ++col) 
            {
                switch (board[row][col]) 
                {
                    case EMPTY:
                        std::cout << ". ";
                        break;
                    case PLAYER_1_PIECE:
                        std::cout << "1 ";
                        break;
                    case PLAYER_1_KING:
                        std::cout << "1K";
                        break;
                    case PLAYER_2_PIECE:
                        std::cout << "2 ";
                        break;
                    case PLAYER_2_KING:
                        std::cout << "2K";
                        break;
                }
            }
            std::cout << "\n";
        }
    }

    bool makeMove() 
    {
        Move move = getMove();
        if (isValidMove(move)) 
        {
            executeMove(move);
            return true;
        }
        else {
            std::cout << "Invalid move. Try again.\n";
            return false;
        }
    }

    Move getMove() 
    {
        Move move;
        std::cout << "Enter move (fromRow fromCol toRow toCol): " << std::endl;
        std::string fromRow = "", fromCol = "", toRow = "", toCol = "";
        //check if input is a single digit
        while (!(isdigit(fromRow[0]) && isdigit(fromCol[0]) && isdigit(toRow[0]) && isdigit(toCol[0])) || 
                (fromRow.length() != 1 && toRow.length() != 1 && fromCol.length() != 1 && toCol.length() != 1))
        {
            std::cout << "(please enter only digits)" << std::endl;
            std::cin >> fromRow >> fromCol >> toRow >> toCol;
        }
        //assign to "move"
        move.fromRow = stoi(fromRow), move.fromCol = stoi(fromCol), move.toRow = stoi(toRow), move.toCol = stoi(toCol);
        return move;
    }

    bool isValidMove(const Move& move) 
    {
        //check if number in range
        if( move.fromCol<=7 && move.fromCol >=0 &&
            move.fromRow <= 7 && move.fromRow >= 0 &&
            move.toCol <= 7 && move.toCol >= 0 &&
            move.toRow <= 7 && move.toRow >= 0)
            {
                int playerDirection = currentPlayer == PLAYER_1 ? -1 : 1;
                Piece opponentKing = currentPlayer == PLAYER_1 ? PLAYER_2_KING : PLAYER_1_KING;
                Piece opponentPiece = currentPlayer == PLAYER_1 ? PLAYER_2_PIECE : PLAYER_1_PIECE;
                switch (board[move.fromRow][move.fromCol]) 
                {
                    case PLAYER_1_PIECE: 
                    case PLAYER_2_PIECE:
                        return 
                            //case not eating move - check that it moves 1 hop in the right direction and that it moves to an empty spot
                            (move.fromRow == move.toRow + playerDirection && abs(move.fromCol - move.toCol) == 1 && board[move.toRow][move.toCol] == EMPTY) ||
                            //case eating move - check that it moves 2 hop in the right direction and that it moves to an empty spot
                            //AND check that it hops over an opponent's piece
                            (move.fromRow == move.toRow + 2 * playerDirection && abs(move.fromCol - move.toCol) == 2 && board[move.toRow][move.toCol] == EMPTY) &&
                            ((board[(move.fromRow + move.toRow) / 2][(move.fromCol + move.toCol)/2] == opponentPiece) ||  
                            (board[(move.fromRow + move.toRow) / 2][(move.fromCol + move.toCol) / 2] == opponentKing))
                            ? true : false;
                   
                    case PLAYER_2_KING:
                    case PLAYER_1_KING:
                        //check that the king moves in the right diection to an empty spot AND that the king doesnt hop over its own pieces
                        return (abs(move.fromRow - move.toRow) == abs(move.fromCol - move.toCol) && board[move.toRow][move.toCol] == EMPTY) &&
                                validKingEatMove(move)
                                ? true : false;
                    
                    default: //case empty
                        return false;
                }
            }else return false;
    }

    void executeMove(const Move& move) 
    {
        //check who is making the move
        if (board[move.fromRow][move.fromCol] == PLAYER_1_PIECE || board[move.fromRow][move.fromCol] == PLAYER_1_KING) 
        {
            //if you move to an edge row OR you were already a king
            if (move.toRow == 7 || board[move.fromRow][move.fromCol] == PLAYER_1_KING)
            {
                board[move.toRow][move.toCol] = PLAYER_1_KING;
            }else{
                board[move.toRow][move.toCol] = PLAYER_1_PIECE;
            }
        }
        else { //must be player 2 considering "move" has already been validated so it cant be empty
            if (move.toRow == 0 || board[move.fromRow][move.fromCol] == PLAYER_2_KING)
            {
                board[move.toRow][move.toCol] = PLAYER_2_KING;
            }else{
                board[move.toRow][move.toCol] = PLAYER_2_PIECE;
            }
        }
        board[move.fromRow][move.fromCol] = EMPTY;
        
        if (abs(move.fromCol - move.toCol) > 1) eat(move);
    }

    void switchPlayer() 
    {
        currentPlayer = (currentPlayer == PLAYER_1) ? PLAYER_2 : PLAYER_1;
    }

    bool isGameOver() 
    {
        int p1PeiceCounter =0, p2PeiceCounter = 0;
        for (const auto& row : board) {
            for (Piece piece : row) {
                if (piece == PLAYER_1_PIECE || piece == PLAYER_1_KING) p1PeiceCounter++;
                else if (piece == PLAYER_2_PIECE || piece == PLAYER_2_KING) p2PeiceCounter++;
            }
        }
        return (p1PeiceCounter && p2PeiceCounter) ? false : true;
    }

    Player getWinner() 
    {
        for (const auto& row : board)
        {
            for (Piece value : row)
            {
                if (std::count(row.begin(), row.end(), PLAYER_1_PIECE) ||
                    std::count(row.begin(), row.end(), PLAYER_1_KING)) 
                {
                    return PLAYER_1;
                }  
            }
        }
        return PLAYER_2;
    }

    bool validKingEatMove(const Move& move) {
        int rowDirection = (move.toRow - move.fromRow) / abs(move.toRow - move.fromRow);
        int colDirection = (move.toCol - move.fromCol) / abs(move.toCol - move.fromCol);

        for (int i = 1; i < abs(move.fromRow - move.toRow); i++)
        {
            switch (board[move.fromRow + rowDirection * i][move.fromCol + colDirection * i]) {
            case PLAYER_1_PIECE:
            case PLAYER_1_KING:
                //if one of the pieces between the kings hop was your own piece
                if (currentPlayer == PLAYER_1) {
                    return false;
                }
                //case player 2
                else {
                    //check that there are no adjacent pieces of the opponent
                    if (board[move.fromRow + rowDirection * (i + 1)][move.fromCol + colDirection * (i + 1)] == EMPTY
                        || (board[move.fromRow + rowDirection * i][move.fromCol + colDirection * i]) == EMPTY) break;
                    else return false;
                }
            case PLAYER_2_PIECE:
            case PLAYER_2_KING:
                if (currentPlayer == PLAYER_2) {
                    return false;
                }
                //case player 1
                else {
                    if (board[move.fromRow + rowDirection * (i + 1)][move.fromCol + colDirection * (i + 1)] == EMPTY ||
                        (board[move.fromRow + rowDirection * i][move.fromCol + colDirection * i]) == EMPTY)  break;
                    else return false;
                }
                //case empty
            default:
                break;
            }
        }
        return true;
    }

    void eat(const Move& move) 
    {
        if (move.fromRow > move.toRow && move.fromCol > move.toCol)
        {
            for (int i = 1; i < abs(move.fromRow - move.toRow); i++)
            {
                board[move.fromRow - i][move.fromCol - i] = EMPTY;
            }
        }
        else if (move.fromRow < move.toRow && move.fromCol > move.toCol)
        {
            for (int i = 1; i < abs(move.fromRow - move.toRow); i++)
            {
                board[move.fromRow + i][move.fromCol - i] = EMPTY;
            }
        }
        else if (move.fromRow > move.toRow && move.fromCol < move.toCol)
        {
            for (int i = 1; i < abs(move.fromRow - move.toRow); i++)
            {
                board[move.fromRow - i][move.fromCol + i] = EMPTY;
            }
        }
        else
        {
            for (int i = 1; i < abs(move.fromRow - move.toRow); i++)
            {
                board[move.fromRow + i][move.fromCol + i] =EMPTY;
            }
        }
    }
};

int main() 
{
    CheckersGame game;
    game.play();

    return 0;
}