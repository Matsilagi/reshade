#pragma once

#include "EffectLexer.hpp"

#include <vector>
#include <string>

namespace ReShade
{
	namespace FX
	{
		class Node abstract
		{
		public:
			enum class Id
			{
				Unknown,

				LValue,
				Literal,
				Unary,
				Binary,
				Intrinsic,
				Conditional,
				Assignment,
				Sequence,
				Call,
				Constructor,
				Swizzle,
				FieldSelection,
				InitializerList,

				Compound,
				DeclaratorList,
				ExpressionStatement,
				If,
				Switch,
				Case,
				For,
				While,
				Return,
				Jump,

				Annotation,
				Variable,
				Struct,
				Function,
				Pass,
				Technique
			};
			
			const Id NodeId;
			Lexer::Location Location;
			
		protected:
			Node(Id id) : NodeId(id), Location()
			{
			}

		private:
			void operator =(const Node &);
		};

		namespace Nodes
		{
			struct Type
			{
				enum class Class
				{
					Void,
					Bool,
					Int,
					Uint,
					Float,
					Sampler1D,
					Sampler2D,
					Sampler3D,
					Texture1D,
					Texture2D,
					Texture3D,
					Struct,
					String
				};
				enum Qualifier
				{
					// Storage
					Extern = 1 << 0,
					Static = 1 << 1,
					Uniform = 1 << 2,
					Volatile = 1 << 3,
					Precise = 1 << 4,
					In = 1 << 5,
					Out = 1 << 6,
					InOut = In | Out,

					// Modifier
					Const = 1 << 8,

					// Interpolation
					Linear = 1 << 10,
					NoPerspective = 1 << 11,
					Centroid = 1 << 12,
					NoInterpolation = 1 << 13,
				};
		
				inline bool IsArray() const
				{
					return this->ArrayLength != 0;
				}
				inline bool IsMatrix() const
				{
					return this->Rows >= 1 && this->Cols > 1;
				}
				inline bool IsVector() const
				{
					return this->Rows > 1 && !IsMatrix();
				}
				inline bool IsScalar() const
				{
					return !IsArray() && !IsMatrix() && !IsVector() && IsNumeric();
				}
				inline bool IsNumeric() const
				{
					return IsBoolean() || IsIntegral() || IsFloatingPoint();
				}
				inline bool IsVoid() const
				{
					return this->BaseClass == Class::Void;
				}
				inline bool IsBoolean() const
				{
					return this->BaseClass == Class::Bool;
				}
				inline bool IsIntegral() const
				{
					return this->BaseClass == Class::Int || this->BaseClass == Class::Uint;
				}
				inline bool IsFloatingPoint() const
				{
					return this->BaseClass == Class::Float;
				}
				inline bool IsTexture() const
				{
					return this->BaseClass >= Class::Texture1D && this->BaseClass <= Class::Texture2D;
				}
				inline bool IsSampler() const
				{
					return this->BaseClass >= Class::Sampler1D && this->BaseClass <= Class::Sampler3D;
				}
				inline bool IsStruct() const
				{
					return this->BaseClass == Class::Struct;
				}
				inline bool HasQualifier(Qualifier qualifier) const
				{
					return (this->Qualifiers & static_cast<unsigned int>(qualifier)) == static_cast<unsigned int>(qualifier);
				}

				Class BaseClass;
				unsigned int Qualifiers;
				unsigned int Rows : 4, Cols : 4;
				int ArrayLength;
				struct Struct *Definition;
			};
	
			struct Statement abstract : public Node
			{
				std::vector<std::string> Attributes;

			protected:
				Statement(Id id) : Node(id)
				{
				}
			};
			struct Expression abstract : public Node
			{
				Type Type;

			protected:
				Expression(Id id) : Node(id)
				{
				}
			};
	
			// Statements
			struct Compound : public Statement
			{
				std::vector<Statement *> Statements;

				Compound() : Statement(Id::Compound)
				{
				}
			};
			struct DeclaratorList : public Statement
			{
				std::vector<struct Variable *> Declarators;

				DeclaratorList() : Statement(Id::DeclaratorList)
				{
				}
			};
			struct ExpressionStatement : public Statement
			{
				Expression *Expression;

				ExpressionStatement() : Statement(Id::ExpressionStatement)
				{
				}
			};
			struct If : public Statement
			{
				Expression *Condition;
				Statement *StatementOnTrue, *StatementOnFalse;
				
				If() : Statement(Id::If)
				{
				}
			};
			struct Case : public Statement
			{
				std::vector<struct Literal *> Labels;
				Statement *Statements;

				Case() : Statement(Id::Case)
				{
				}
			};
			struct Switch : public Statement
			{
				Expression *Test;
				std::vector<Case *> Cases;

				Switch() : Statement(Id::Switch)
				{
				}
			};
			struct For : public Statement
			{
				Statement *Initialization;
				Expression *Condition, *Increment;
				Statement *Statements;
				
				For() : Statement(Id::For)
				{
				}
			};
			struct While : public Statement
			{
				bool DoWhile;
				Expression *Condition;
				Statement *Statements;
				
				While() : Statement(Id::While), DoWhile(false)
				{
				}
			};
			struct Return : public Statement
			{
				bool Discard;
				Expression *Value;
				
				Return() : Statement(Id::Return), Discard(false), Value(nullptr)
				{
				}
			};
			struct Jump : public Statement
			{
				enum Mode
				{
					Break,
					Continue
				};

				Mode Mode;
				
				Jump() : Statement(Id::Jump), Mode(Break)
				{
				}
			};
	
			// Expressions
			struct LValue : public Expression
			{
				const struct Variable *Reference;
				
				LValue() : Expression(Id::LValue), Reference(nullptr)
				{
				}
			};
			struct Literal : public Expression
			{
				union Value
				{
					int Int[16];
					unsigned int Uint[16];
					float Float[16];
				};
		
				Value Value;
				std::string StringValue;
				
				Literal() : Expression(Id::Literal)
				{
					memset(&this->Value, 0, sizeof(Value));
				}
			};
			struct Unary : public Expression
			{
				enum class Op
				{
					None,

					Negate,
					BitwiseNot,
					LogicalNot,
					Increase,
					Decrease,
					PostIncrease,
					PostDecrease,
					Cast,
				};
	
				Op Operator;
				Expression *Operand;

				Unary() : Expression(Id::Unary), Operator(Op::None), Operand(nullptr)
				{
				}
			};
			struct Binary : public Expression
			{
				enum class Op
				{
					None,

					Add,
					Subtract,
					Multiply,
					Divide,
					Modulo,
					Less,
					Greater,
					LessOrEqual,
					GreaterOrEqual,
					Equal,
					NotEqual,
					LeftShift,
					RightShift,
					BitwiseOr,
					BitwiseXor,
					BitwiseAnd,
					LogicalOr,
					LogicalAnd,
					ElementExtract
				};

				Op Operator;
				Expression *Operands[2];

				Binary() : Expression(Id::Binary), Operator(Op::None), Operands()
				{
				}
			};
			struct Intrinsic : public Expression
			{
				enum class Op
				{
					None,

					Abs,
					Acos,
					All,
					Any,
					BitCastInt2Float,
					BitCastUint2Float,
					Asin,
					BitCastFloat2Int,
					BitCastFloat2Uint,
					Atan,
					Atan2,
					Ceil,
					Clamp,
					Cos,
					Cosh,
					Cross,
					PartialDerivativeX,
					PartialDerivativeY,
					Degrees,
					Determinant,
					Distance,
					Dot,
					Exp,
					Exp2,
					FaceForward,
					Floor,
					Frac,
					Frexp,
					Fwidth,
					Ldexp,
					Length,
					Lerp,
					Log,
					Log10,
					Log2,
					Mad,
					Max,
					Min,
					Modf,
					Mul,
					Normalize,
					Pow,
					Radians,
					Rcp,
					Reflect,
					Refract,
					Round,
					Rsqrt,
					Saturate,
					Sign,
					Sin,
					SinCos,
					Sinh,
					SmoothStep,
					Sqrt,
					Step,
					Tan,
					Tanh,
					Tex2D,
					Tex2DBias,
					Tex2DFetch,
					Tex2DGather,
					Tex2DGatherOffset,
					Tex2DLevel,
					Tex2DLevelOffset,
					Tex2DOffset,
					Tex2DSize,
					Transpose,
					Trunc,
				};

				Op Operator;
				Expression *Arguments[3];

				Intrinsic() : Expression(Id::Intrinsic), Operator(Op::None), Arguments()
				{
				}
			};
			struct Conditional : public Expression
			{
				Expression *Condition;
				Expression *ExpressionOnTrue, *ExpressionOnFalse;
				
				Conditional() : Expression(Id::Conditional), Condition(nullptr), ExpressionOnTrue(nullptr), ExpressionOnFalse(nullptr)
				{
				}
			};
			struct Assignment : public Expression
			{
				enum class Op
				{
					None,
					Add,
					Subtract,
					Multiply,
					Divide,
					Modulo,
					BitwiseAnd,
					BitwiseOr,
					BitwiseXor,
					LeftShift,
					RightShift
				};
		
				Op Operator;
				Expression *Left, *Right;
				
				Assignment() : Expression(Id::Assignment), Operator(Op::None)
				{
				}
			};
			struct Sequence : public Expression
			{
				std::vector<Expression *> Expressions;
				
				Sequence() : Expression(Id::Sequence)
				{
				}
			};
			struct Call : public Expression
			{
				std::string CalleeName;
				const struct Function *Callee;
				std::vector<Expression *> Arguments;
				
				Call() : Expression(Id::Call), Callee(nullptr)
				{
				}
			};
			struct Constructor : public Expression
			{
				std::vector<Expression *> Arguments;
				
				Constructor() : Expression(Id::Constructor)
				{
				}
			};
			struct Swizzle : public Expression
			{
				Expression *Operand;
				signed char Mask[4];

				Swizzle() : Expression(Id::Swizzle),  Operand(nullptr)
				{
					this->Mask[0] = this->Mask[1] = this->Mask[2] = this->Mask[3] = -1;
				}
			};
			struct FieldSelection : public Expression
			{
				Expression *Operand;
				Variable *Field;

				FieldSelection() : Expression(Id::FieldSelection),  Operand(nullptr), Field(nullptr)
				{
				}
			};
			struct InitializerList : public Expression
			{
				std::vector<Expression *> Values;

				InitializerList() : Expression(Id::InitializerList)
				{
				}
			};

			// Declarations
			struct Annotation : public Node
			{
				std::string Name;
				Literal *Value;
				
				Annotation() : Node(Id::Annotation)
				{
				}
			};
			struct Variable : public Node
			{
				struct Properties
				{
					enum : unsigned int
					{
						NONE = 0,

						R8 = 50,
						R32F = 114,
						RG8 = 51,
						RGBA8 = 32,
						RGBA16 = 36,
						RGBA16F = 113,
						RGBA32F = 116,
						DXT1 = 827611204,
						DXT3 = 861165636,
						DXT5 = 894720068,
						LATC1 = 826889281,
						LATC2 = 843666497,

						POINT = 1,
						LINEAR,
						ANISOTROPIC,

						WRAP = 1,
						REPEAT = 1,
						MIRROR,
						CLAMP,
						BORDER,
					};

					Properties() : Texture(nullptr), Width(1), Height(1), Depth(1), MipLevels(1), Format(RGBA8), SRGBTexture(false), AddressU(CLAMP), AddressV(CLAMP), AddressW(CLAMP), MinFilter(LINEAR), MagFilter(LINEAR), MipFilter(LINEAR), MaxAnisotropy(1), MinLOD(-FLT_MAX), MaxLOD(FLT_MAX), MipLODBias(0.0f)
					{
					}

					const Variable *Texture;
					unsigned int Width, Height, Depth, MipLevels;
					unsigned int Format;
					bool SRGBTexture;
					unsigned int AddressU, AddressV, AddressW, MinFilter, MagFilter, MipFilter;
					unsigned int MaxAnisotropy;
					float MinLOD, MaxLOD, MipLODBias;
				};

				Type Type;
				std::string Name;
				std::vector<Annotation> Annotations;
				std::string Semantic;
				Properties Properties;
				Expression *Initializer;
				
				Variable() : Node(Id::Variable), Initializer(nullptr)
				{
				}
			};
			struct Struct : public Node
			{
				std::string Name;
				std::vector<Variable *> Fields;
				
				Struct() : Node(Id::Struct)
				{
				}
			};
			struct Function : public Node
			{
				Type ReturnType;
				std::string Name;
				std::vector<Variable *> Parameters;
				std::string ReturnSemantic;
				Compound *Definition;
				
				Function() : Node(Id::Function), Definition(nullptr)
				{
				}
			};
			struct Pass : public Node
			{
				struct States
				{
					enum : unsigned int
					{
						NONE = 0,

						ZERO = 0,
						ONE = 1,
						SRCCOLOR,
						INVSRCCOLOR,
						SRCALPHA,
						INVSRCALPHA,
						DESTALPHA,
						INVDESTALPHA,
						DESTCOLOR,
						INVDESTCOLOR,

						ADD = 1,
						SUBTRACT,
						REVSUBTRACT,
						MIN,
						MAX,

						KEEP = 1,
						REPLACE = 3,
						INVERT,
						INCRSAT,
						DECRSAT,
						INCR,
						DECR,

						NEVER = 1,
						LESS,
						EQUAL,
						LESSEQUAL,
						GREATER,
						NOTEQUAL,
						GREATEREQUAL,
						ALWAYS
					};

					States() : RenderTargets(), VertexShader(nullptr), PixelShader(nullptr), SRGBWriteEnable(false), BlendEnable(false), DepthEnable(false), StencilEnable(false), RenderTargetWriteMask(0xF), DepthWriteMask(1), StencilReadMask(0xFF), StencilWriteMask(0xFF), BlendOp(ADD), BlendOpAlpha(ADD), SrcBlend(ONE), DestBlend(ZERO), DepthFunc(LESS), StencilFunc(ALWAYS), StencilRef(0), StencilOpPass(KEEP), StencilOpFail(KEEP), StencilOpDepthFail(KEEP)
					{
					}

					const Variable *RenderTargets[8];
					const Function *VertexShader, *PixelShader;
					bool SRGBWriteEnable, BlendEnable, DepthEnable, StencilEnable;
					unsigned char RenderTargetWriteMask, DepthWriteMask, StencilReadMask, StencilWriteMask;
					unsigned int BlendOp, BlendOpAlpha, SrcBlend, DestBlend, DepthFunc, StencilFunc, StencilRef, StencilOpPass, StencilOpFail, StencilOpDepthFail;
				};

				std::string Name;
				std::vector<Annotation> Annotations;
				States States;
				
				Pass() : Node(Id::Pass)
				{
				}
			};
			struct Technique : public Node
			{
				std::string Name;
				std::vector<Annotation> Annotations;
				std::vector<Pass *> Passes;
				
				Technique() : Node(Id::Technique)
				{
				}
			};
		}
	}
}