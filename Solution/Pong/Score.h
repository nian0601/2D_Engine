#pragma once

class Score
{
public:
	Score();

	void Render();

	void AddLeftPlayerScore() { ++myLeftPlayerScore; }
	void AddRightPlayerScore() { ++myRightPlayerScore; }
	void Reset() { myLeftPlayerScore = 0; myRightPlayerScore = 0; }

	bool GameIsOver() const { return myLeftPlayerScore >= 3 || myRightPlayerScore >= 3; }

private:
	int myLeftPlayerScore;
	int myRightPlayerScore;
};
