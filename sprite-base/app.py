from flask import Flask, request, jsonify, render_template
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy.sql.expression import func

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///sprites.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

class Sprite(db.Model):
    name = db.Column(db.String(80), primary_key=True, unique=True, nullable=False)
    data = db.Column(db.LargeBinary, nullable=False)

    def __repr__(self):
        return f'<Sprite {self.name}>'

with app.app_context():
    db.create_all()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/test_decoder')
def test_decoder_page():
    return render_template('test_decoder.html')

@app.route('/sprite/<string:name>', methods=['POST'])
def add_sprite(name):
    if Sprite.query.get(name):
        return jsonify({'error': 'Sprite with this name already exists'}), 409

    data = request.get_data()
    if not data:
        return jsonify({'error': 'No data provided'}), 400

    new_sprite = Sprite(name=name, data=data)
    db.session.add(new_sprite)
    db.session.commit()
    return jsonify({'message': f'Sprite {name} created successfully'}), 201

@app.route('/sprite/<string:name>', methods=['GET'])
def get_sprite(name):
    sprite = Sprite.query.get(name)
    if sprite:
        return sprite.data, 200, {'Content-Type': 'application/octet-stream'}
    return jsonify({'error': 'Sprite not found'}), 404

@app.route('/sprites', methods=['GET', 'DELETE'])
def handle_sprites():
    if request.method == 'GET':
        sprites = Sprite.query.all()
        sprite_names = [sprite.name for sprite in sprites]
        return jsonify(sprite_names)
    
    if request.method == 'DELETE':
        try:
            num_rows_deleted = db.session.query(Sprite).delete()
            db.session.commit()
            return jsonify({'message': f'{num_rows_deleted} sprites deleted.'}), 200
        except Exception as e:
            db.session.rollback()
            return jsonify({'error': str(e)}), 500

@app.route('/sprite/random', methods=['GET'])
def get_random_sprite():
    sprite = Sprite.query.order_by(func.random()).first()
    if sprite:
        return sprite.data, 200, {'Content-Type': 'application/octet-stream'}
    return jsonify({'error': 'No sprites found in the database'}), 404

if __name__ == '__main__':
    app.run(debug=True) 