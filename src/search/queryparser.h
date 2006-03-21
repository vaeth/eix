#include <iostream>
class Matchatom;
class PropertyMatcher;
class StringMatcher;

/*
	new Matchatom
	need_logical_operator = false
	for each option in list
		# precheck for logical operator
		if need_logical_operator:
			need_logical_operator = false
			if AND:
				Matchatom->and = new Matchatom
				Matchatom = Matchatom->and
				continue
			else if OR
				Matchatom->and = new Matchatom
				Matchatom = Matchatom->and
				continue

			Matchatom->or = new Matchatom
			Matchatom = Matchatom->and

		switch option:
			case !:
				Matchatom->negate = true
				break

			case [property-designator]:
				new PropertyMatcher
				option-iterator = PropertyMatcher->parse (option-iterator)
				need_logical_operator = true
				break

			default:
				new StringMatcher
				option-iterator = StringMatcher->parse (option-iterator)
				need_logical_operator = true
				break
*/

class QueryParser {

	private:
		typedef vector<Parameter>::iterator OptionIterator;

		/** Parse the complete list of parameters into a chain of NODEs. */
		Matchatom *parse(OptionIterator begin, OptionIterator end) {
			Matchatom *root = new Matchatom();
			Matchatom *current = root;

			bool need_logical_operator = false;
			for(; begin != end; ++begin)
			{
				// check for logical operator
				if(need_logical_operator)
				{
					try {
						current->finalize();
					}
					catch(string s) {
						cout << s << endl;
						exit(1);
					}

					// check if current is satisfied
					need_logical_operator = false
					if(*begin == 'a')
					{
						current->next_and = new Matchatom();
						current = current->next_and;
						continue;
					}
					else if(*begin == 'o')
					{
						current->next_or = new Matchatom();
						current = current->next_or;
						continue;
					}

					current->or = new Matchatom();
					current = current->next_or;
				}

				switch(*begin)
				{
					case '!':
						current->negate = true;
						break;
					case DUP_VERSIONS:
						current->dup_versions = true;
						break;
					case INSTALLED:
						current->installed_versions = true;
						break;

					default:
						new StringMatcher
						option-iterator = StringMatcher->parse (option-iterator)
						need_logical_operator = true
						break
				}
			}
			// check if current is satisfied
		}

#if 0
		/** Parse a StringTest */
		StringTest *parse_stringtest(OptionIterator &begin, OptionIterator end) {
		}

		/** Parse a PropertyTest */
		PropertyTest *parse_propertytest(OptionIterator &begin, OptionIterator end) {
			current_node->set()
		}
#endif
};
