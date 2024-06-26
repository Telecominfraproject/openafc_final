"""empty message

Revision ID: a0977ae9d7f7
Revises: 41f24c9fbec8
Create Date: 2023-02-13 22:02:18.057538

"""

# revision identifiers, used by Alembic.
revision = 'a0977ae9d7f7'
down_revision = '41f24c9fbec8'

from alembic import op
import sqlalchemy as sa
from sqlalchemy.dialects import postgresql

def upgrade():
    # ### commands auto generated by Alembic - please adjust! ###
    op.drop_table('blacklist_tokens')
    # ### end Alembic commands ###


def downgrade():
    # ### commands auto generated by Alembic - please adjust! ###
    op.create_table('blacklist_tokens',
    sa.Column('id', sa.INTEGER(), autoincrement=True, nullable=False),
    sa.Column('token', sa.VARCHAR(length=500), autoincrement=False, nullable=False),
    sa.Column('blacklisted_on', postgresql.TIMESTAMP(), autoincrement=False, nullable=False),
    sa.PrimaryKeyConstraint('id', name='blacklist_tokens_pkey'),
    sa.UniqueConstraint('token', name='blacklist_tokens_token_key')
    )
    # ### end Alembic commands ###
