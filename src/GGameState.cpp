#include "Game.h"

static const TInt ANIMATE_TIME = 3;
static const TInt BLINK_TIME   = 2;

static const TInt PLAYERSTATE_CONTROL  = 0,
                  PLAYERSTATE_GAMEOVER = 1;

static const TUint8 GRID_COLOR = 253;

class GPlayerSprite : public BSprite {
public:
  GPlayerSprite() : BSprite(0, PLAYER_SLOT) {
    this->flags = SFLAG_RENDER;
    Randomize();
  }

  ~GPlayerSprite() override = default;

public:
  void Randomize() {
    mBlocks[0] = TUint8(Random() & 1);
    mBlocks[1] = TUint8(Random() & 1);
    mBlocks[2] = TUint8(Random() & 1);
    mBlocks[3] = TUint8(Random() & 1);
    this->x  = PLAYER_X;
    this->y  = PLAYER_Y;
    this->vy = 0;
  }

  void RotateLeft() {
    TUint8 save = mBlocks[0];

    mBlocks[0] = mBlocks[1];
    mBlocks[1] = mBlocks[3];
    mBlocks[3] = mBlocks[2];
    mBlocks[2] = save;
  }

  void RotateRight() {
    TUint8 save = mBlocks[0];

    mBlocks[0] = mBlocks[2];
    mBlocks[2] = mBlocks[3];
    mBlocks[3] = mBlocks[1];
    mBlocks[1] = save;
  }

  void Animate() override {
    mBlinkTimer--;
    if (mBlinkTimer < 0) {
      mBlinkTimer = BLINK_TIME;
    }

  }

  TBool Render(BViewPort *aVP) override {
    if (flags & SFLAG_RENDER) {
      BBitmap *bm = gDisplay.renderBitmap;
      TInt    xx  = TInt(x + .5);
      TInt    yy  = TInt(y + .5);

      bm->DrawSprite(gViewPort, PLAYER_SLOT, mBlocks[0], xx, yy);
      bm->DrawSprite(gViewPort, PLAYER_SLOT, mBlocks[1], xx + 16, yy);
      bm->DrawSprite(gViewPort, PLAYER_SLOT, mBlocks[2], xx, yy + 16);
      bm->DrawSprite(gViewPort, PLAYER_SLOT, mBlocks[3], xx + 16, yy + 16);
    }
    return ETrue;
  }

  void Swap(GPlayerSprite *aOther) {
    TUint8 save;

    save = aOther->mBlocks[0];
    aOther->mBlocks[0] = this->mBlocks[0];
    this->mBlocks[0]   = save;

    save = aOther->mBlocks[1];
    aOther->mBlocks[1] = this->mBlocks[1];
    this->mBlocks[1]   = save;

    save = aOther->mBlocks[2];
    aOther->mBlocks[2] = this->mBlocks[2];
    this->mBlocks[2]   = save;

    save = aOther->mBlocks[3];
    aOther->mBlocks[3] = this->mBlocks[3];
    this->mBlocks[3]   = save;
  }

public:
  TInt   mBlinkTimer;
  TUint8 mBlocks[4];
};;

class GGameProcess : public BProcess {
public:
  GGameProcess(GGameState *aGameState) : BProcess() {
    mGameState  = aGameState;
    mState      = PLAYERSTATE_CONTROL;
    mBlinkTimer = BLINK_TIME;

    mSprite = new GPlayerSprite();
    gGameEngine->AddSprite(mSprite);
  }

  virtual ~GGameProcess() {
  }

public:
  TBool RunBefore() {
    return ETrue;
  }

  TInt BoardRow() {
    return mGameState->mBoardY + TInt(mSprite->y - BOARD_Y) / 16;

  }

  TInt BoardCol() {
    return mGameState->mBoardX + TInt(mSprite->x - BOARD_X) / 16;
  }

  TBool CanDrop() {
    TInt row = BoardRow(),
         col = BoardCol();

    if (mGameState->mGameBoard[row][col] != 255 || mGameState->mGameBoard[row][col + 1] != 255 ||
        mGameState->mGameBoard[row + 1][col] != 255 || mGameState->mGameBoard[row + 1][col + 1] != 255) {
      return EFalse;
    }
    return ETrue;
  }

  void Drop() {
    if (!CanDrop()) {
      // TODO: Jay - make a "cant do that!" sound here
      return;
    }

    TInt row = BoardRow(),
         col = BoardCol();

    mGameState->mGameBoard[row][col]         = mSprite->mBlocks[0];
    mGameState->mGameBoard[row][col + 1]     = mSprite->mBlocks[1];
    mGameState->mGameBoard[row + 1][col]     = mSprite->mBlocks[2];
    mGameState->mGameBoard[row + 1][col + 1] = mSprite->mBlocks[3];
    mSprite->Randomize();
  }

  TBool StateGameOver() {
    mSprite->flags &= ~SFLAG_RENDER;
    if (gControls.WasPressed(BUTTON_START)) {
      gGameEngine->RemoveSprite(mSprite);
      delete mSprite;
      mSprite = ENull;

      gGameEngine->SetState(GAME_STATE_GAMEOVER);
      return EFalse;
    }

  }

  void StateControl() {
    if (gControls.WasPressed(BUTTONA)) {
      mSprite->RotateLeft();
    } else if (gControls.WasPressed(BUTTONB)) {
      mSprite->RotateRight();
    } else if (gControls.WasPressed(JOYLEFT)) {
      mSprite->x -= 16;
      if (mSprite->x < PLAYER_X_MIN) {
        mSprite->x          = PLAYER_X_MIN;
        mGameState->mBoardX = MAX(mGameState->mBoardX - 1, 0);
      }
    } else if (gControls.WasPressed(JOYRIGHT)) {
      mSprite->x += 16;
      if (mSprite->x > PLAYER_X_MAX) {
        mSprite->x          = PLAYER_X_MAX;
        mGameState->mBoardX = MIN(mGameState->mBoardX + 1, BOARD_X_MAX);
      }
    } else if (gControls.WasPressed(JOYDOWN)) {
      mSprite->y += 16;
      if (mSprite->y > PLAYER_Y_MAX) {
        mSprite->y          = PLAYER_Y_MAX;
        mGameState->mBoardY = MIN(mGameState->mBoardY + 1, BOARD_Y_MAX);
      }
    } else if (gControls.WasPressed(JOYUP)) {
      mSprite->y -= 16;
      if (mSprite->y < PLAYER_Y_MIN) {
        mSprite->y          = PLAYER_Y_MIN;
        mGameState->mBoardY = MAX(mGameState->mBoardY - 1, 0);
      }
    } else if (gControls.WasPressed(BUTTON_SELECT)) {
      Drop();
      mGameState->Combine();
      if (mGameState->IsGameOver()) {
        mState = PLAYERSTATE_GAMEOVER;
        mGameState->mGameOver = ETrue;
      }
    }
    mBlinkTimer--;
    if (mBlinkTimer < 0) {
      mBlinkTimer = BLINK_TIME;
      if (!CanDrop()) {
        mSprite->flags ^= SFLAG_RENDER;
      }
    } else if (CanDrop()) {
      mSprite->flags |= SFLAG_RENDER;
    }
  }

  TBool RunAfter() {
    switch (mState) {
      case PLAYERSTATE_CONTROL:
        StateControl();
        break;
      case PLAYERSTATE_GAMEOVER:
        return StateGameOver();
      default:
        Panic("Invalid mState in RunAfter\n");
        break;
    }
    return ETrue;
  }

  TInt          mState;
  TInt          mBlinkTimer;
  GPlayerSprite *mSprite;
  GGameState    *mGameState;
  GPlayerSprite *mNextSprite1;
  GPlayerSprite *mNextSprite2;
};

GGameState::GGameState(GGameEngine *aGameEngine) : BPlayfield(aGameEngine) {
  mAnimationTimer = ANIMATE_TIME;
  mGameEngine     = aGameEngine;
  mLevel          = 1;
  mGameOver       = EFalse;
  LoadLevel();
}

GGameState::~GGameState() {
  gResourceManager.ReleaseBitmapSlot(BKG_SLOT);
  gResourceManager.ReleaseBitmapSlot(BKG2_SLOT);
}

void GGameState::LoadLevel() {
  switch (mLevel) {
    case 1:
    default:
      gResourceManager.LoadBitmap(LEVEL1_SPRITES_BMP, PLAYER_SLOT, IMAGE_16x16);
      gResourceManager.LoadBitmap(LEVEL1_BKG1_BMP, BKG_SLOT, IMAGE_ENTIRE);
      gResourceManager.LoadBitmap(LEVEL1_BKG2_BMP, BKG2_SLOT, IMAGE_ENTIRE);
      break;
  }
  mBackground1       = gResourceManager.GetBitmap(BKG_SLOT);
  mBackground2       = gResourceManager.GetBitmap(BKG2_SLOT);
  mCurrentBackground = mBackground1;

  mBackground1->SetColor(GRID_COLOR, 255, 255, 255);
  mBackground2->SetColor(GRID_COLOR, 255, 255, 255);
//  gDisplay.SetPalette(gResourceManager.GetBitmap(PLAYER_SLOT));
  gDisplay.SetPalette(mBackground1);
  mGameProcess = new GGameProcess(this);
  mGameEngine->AddProcess((BProcess *) mGameProcess);

  mBoardX = mBoardY = 0;

  Clear();

  // draw grid on backgrounds
  for (TInt row = 0; row < VISIBLE_BOARD_ROWS + 1; row++) {
    mBackground1->DrawFastHLine(ENull, BOARD_X, BOARD_Y + row * 16, VISIBLE_BOARD_COLS * 16, GRID_COLOR);
    mBackground2->DrawFastHLine(ENull, BOARD_X, BOARD_Y + row * 16, VISIBLE_BOARD_COLS * 16, GRID_COLOR);
  }
  for (TInt col = 0; col < VISIBLE_BOARD_COLS + 1; col++) {
    mBackground1->DrawFastVLine(ENull, BOARD_X + col * 16, BOARD_Y, VISIBLE_BOARD_ROWS * 16, GRID_COLOR);
    mBackground2->DrawFastVLine(ENull, BOARD_X + col * 16, BOARD_Y, VISIBLE_BOARD_ROWS * 16, GRID_COLOR);
  }

  mTextColor = 0;
}

void GGameState::Clear() {
  for (TInt row = 0; row < BOARD_ROWS; row++) {
    for (TInt col = 0; col < BOARD_COLS; col++) {
      mGameBoard[row][col] = 255;
    }
  }
}

void GGameState::Animate() {
  if (--mAnimationTimer <= 0) {
    mCurrentBackground = mCurrentBackground == mBackground1 ? mBackground2 : mBackground1;
    mAnimationTimer    = ANIMATE_TIME;
  }

  mTextColor += 1;
  mTextColor %= 64;
  gDisplay.renderBitmap->SetColor(COLOR_TEXT, 0, 192 + mTextColor, 192 + mTextColor);
}

void GGameState::Combine() {
  TUint32 accumulated_score = 0;
  TUint8  quad[4];

  for (TInt row = 0; row < BOARD_ROWS - 1; row++) {
    for (TInt col = 0; col < BOARD_COLS - 1; col++) {
      if (!GetQuad(row, col, quad)) {
        continue;
      }
      if (quad[0] > 1 || quad[1] > 1 || quad[2] > 1 || quad[3] > 1) {
        continue;
      }

      mGameBoard[row][col] |= 8;
      mGameBoard[row][col + 1] |= 8;
      mGameBoard[row + 1][col] |= 8;
      mGameBoard[row + 1][col + 1] |= 8;
      TInt      score  = 1;
      // look right
      for (TInt right  = col + 1; right < BOARD_COLS; right++) {
        if (!GetQuad(row, right, quad)) {
          break;
        }
        mGameBoard[row][right] |= 8;
        mGameBoard[row + 1][right] |= 8;
        score *= 2;
      }
      // look down
      for (TInt bottom = row + 1; bottom < BOARD_ROWS; bottom++) {
        if (!GetQuad(bottom, col, quad)) {
          break;
        }
        mGameBoard[bottom][col] |= 8;
        mGameBoard[bottom + 1][col] |= 8;
        score *= 2;
      }
      accumulated_score += score;
    }
  }
  mPoints += accumulated_score;
  mLastScore.FromUint32(0);
  TBCD p;
  p.FromUint32(accumulated_score);
  mScore.Add(p);
  printf("Score: %d\n", accumulated_score);
}

TBool GGameState::IsGameOver() {
  for (TInt row = 0; row < BOARD_ROWS - 1; row++) {
    for (TInt col = 0; col < BOARD_COLS - 1; col++) {
      if (mGameBoard[row][col] == 255 && mGameBoard[row][col + 1] == 255 && mGameBoard[row + 1][col] == 255 &&
          mGameBoard[row + 1][col + 1] == 255) {
        return EFalse;
      }
    }
  }
  return ETrue;
}

void GGameState::Render() {
  gDisplay.renderBitmap->CopyPixels(mCurrentBackground);
  BBitmap *bm = gDisplay.renderBitmap;

  // render the board
  TInt      y   = BOARD_Y;
  for (TInt row = 0; row < VISIBLE_BOARD_ROWS; row++) {
    TInt      x   = BOARD_X;
    for (TInt col = 0; col < VISIBLE_BOARD_COLS; col++) {
      TUint8 v = mGameBoard[mBoardY + row][mBoardX + col];
      if (v != 255) {
        bm->DrawSprite(gViewPort, PLAYER_SLOT, v, x + col * 16, y + row * 16);
      }
    }
  }

#if 0
  // render the red circles/dots at the bottom
  for (TInt i = 0, j = mBoardX; i < VISIBLE_BOARD_COLS; i++, j++) {
    if (j & 1) {
      bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_BEAT_ON, BOARD_X + i * 16, SCREEN_HEIGHT - 16);
      bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_BEAT_ON, BOARD_X + i * 16, BOARD_Y - 16);
    } else {
      bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_BEAT_OFF, BOARD_X + i * 16, SCREEN_HEIGHT - 16);
      bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_BEAT_OFF, BOARD_X + i * 16, BOARD_Y - 16);
    }
  }
#endif
  for (TInt i = 0, j = mBoardY; i < VISIBLE_BOARD_ROWS; i++, j++) {
//    if (j & 1) {
//      bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_BEAT_ON, BOARD_X - 16, BOARD_Y + i * 16);
//      bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_BEAT_ON, BOARD_X + VISIBLE_BOARD_COLS * 16, BOARD_Y + i * 16);
//    } else {
//      bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_BEAT_OFF, BOARD_X - 16, BOARD_Y + i * 16);
    bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_BEAT_OFF, BOARD_X + VISIBLE_BOARD_COLS * 16, BOARD_Y + i * 16);
//    }
  }

  // Score
//  bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_SCORE, SCREEN_WIDTH - 40 - 8, 8);
//  bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_SCORE + 1, SCREEN_WIDTH - 40 + 8, 8);
//  bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_SCORE + 2, SCREEN_WIDTH - 40 + 24, 8);
//  y = 8 + 16;
  bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_SCORE, 8, 0);
  bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_SCORE + 1, 24, 0);
  bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_SCORE + 2, 40, 0);
  TInt      x = 56;
  for (TInt i = 0; i < 7; i++) {
    TInt v = (mScore.mValue >> ((7 - i) * 4)) & 0x0f;
//    if (v) {
    bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_NUM0 + v, x, 0);
//    }
    x += 8;
  }
  TInt      v = mScore.mValue & 0x0f;
  bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_NUM0 + v, x, 0);

  if (mGameOver) {
    x = BOARD_X + VISIBLE_BOARD_COLS*16+32;
    bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_GAMEOVER,  x, BOARD_Y);
    bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_GAMEOVER+1,  x+16, BOARD_Y);
    bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_GAMEOVER+2,  x+32, BOARD_Y);
    bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_GAMEOVER+3,  x+48, BOARD_Y);
    bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_GAMEOVER2, x, BOARD_Y + 16);
    bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_GAMEOVER2+1, x+16, BOARD_Y + 16);
    bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_GAMEOVER2+2, x+32, BOARD_Y + 16);
    bm->DrawSprite(gViewPort, PLAYER_SLOT, IMG_GAMEOVER2+3, x+48, BOARD_Y + 16);
  }
}

