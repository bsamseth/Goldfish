
#include "types.h"
#include "move.h"

Move::Move(Square from, Square to, PieceType promotionType, unsigned specialFlag) {
    m_Move = ((specialFlag & 0x3) << 14) | (((promotionType-2) & 0x3) << 12) | ((from & 0x3f) << 6) | (to & 0x3f);
}

Move::Move(Square from, Square to) {
    m_Move = ((0 & 0x3) << 14) | ((0 & 0x3) << 12) | ((from & 0x3f) << 6) | (to & 0x3f);
}

Square Move::getTo() const {
    return Square(m_Move & 0x3f);
}
Square Move::getFrom() const {
    return Square((m_Move >> 6) & 0x3f);
}
unsigned Move::getFlags() const {
  return (m_Move >> 12) & 0x0f;
}
void Move::setTo(Square to) {
  m_Move = m_Move & ~0x3f;
  m_Move = m_Move | (to & 0x3f);
}
void Move::setFrom(Square from) {
  m_Move = m_Move & ~(0x3f << 6);
  m_Move = m_Move | ((from & 0x3f) << 6);
}
