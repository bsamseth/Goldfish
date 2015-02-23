
#include "types.h"
#include "move.h"

Move::Move(Square from, Square to, MoveFlag moveflag) {
    m_Move = ((moveflag & 0x0f) << 12) | ((to & 0x3f) << 6) | (from & 0x3f);
}

Move::Move(Square from, Square to) {
    m_Move = ((QUIET_MOVE & 0x0f) << 12) | ((to & 0x3f) << 6) | (from & 0x3f);
}

unsigned Move::getInteger() {
    return m_Move;
}

Square Move::getFrom() const {
    return Square(m_Move & 0x3f);
}
Square Move::getTo() const {
    return Square((m_Move >> 6) & 0x3f);
}
MoveFlag Move::getFlag() const {
    return (MoveFlag)((m_Move >> 12) & 0x0f);
}
void Move::setTo(Square to) {
  m_Move = m_Move & ~0x3f;
  m_Move = m_Move | (to & 0x3f);
}
void Move::setFrom(Square from) {
  m_Move = m_Move & ~(0x3f << 6);
  m_Move = m_Move | ((from & 0x3f) << 6);
}
