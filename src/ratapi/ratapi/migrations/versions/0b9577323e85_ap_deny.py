"""empty message

Revision ID: 0b9577323e85
Revises: a0977ae9d7f7
Create Date: 2023-04-14 20:10:00.561731

"""

# revision identifiers, used by Alembic.
revision = '0b9577323e85'
down_revision = 'a0977ae9d7f7'

from alembic import op
import sqlalchemy as sa


def upgrade():
    # ### commands auto generated by Alembic - please adjust! ###
    op.create_table('aaa_org',
    sa.Column('id', sa.Integer(), autoincrement=True, nullable=False),
    sa.Column('name', sa.String(length=50), nullable=False),
    sa.PrimaryKeyConstraint('id'),
    sa.UniqueConstraint('name')
    )
    op.create_table('aaa_ruleset',
    sa.Column('id', sa.Integer(), autoincrement=True, nullable=False),
    sa.Column('name', sa.String(length=50), nullable=False),
    sa.PrimaryKeyConstraint('id'),
    sa.UniqueConstraint('name')
    )
    op.create_table('access_point_deny',
    sa.Column('id', sa.Integer(), autoincrement=True, nullable=False),
    sa.Column('serial_number', sa.String(length=64), nullable=True),
    sa.Column('certification_id', sa.String(length=64), nullable=True),
    sa.Column('org_id', sa.Integer(), nullable=True),
    sa.Column('ruleset_id', sa.Integer(), nullable=True),
    sa.ForeignKeyConstraint(['org_id'], ['aaa_org.id'], ondelete='CASCADE'),
    sa.ForeignKeyConstraint(['ruleset_id'], ['aaa_ruleset.id'], ondelete='CASCADE'),
    sa.PrimaryKeyConstraint('id')
    )
    op.create_index(op.f('ix_access_point_deny_serial_number'), 'access_point_deny', ['serial_number'], unique=False)
    # ### end Alembic commands ###


def downgrade():
    # ### commands auto generated by Alembic - please adjust! ###
    op.drop_index(op.f('ix_access_point_deny_serial_number'), table_name='access_point_deny')
    op.drop_table('access_point_deny')
    op.drop_table('aaa_org')
    op.drop_table('aaa_ruleset')
    # ### end Alembic commands ###