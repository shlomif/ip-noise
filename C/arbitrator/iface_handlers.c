static int ip_noise_arbitrator_iface_handler_new_chain(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_new_state(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_move_probs(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_source(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_dest(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_protocol(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_tos(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_length_min(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_length_max(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_which_length(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_drop_delay_prob(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_delay_type(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_split_linear_points(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_lambda(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_time_factor(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_stable_delay_prob(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_clear_all(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_end_connection(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);



#define NUM_OPERATIONS 18
static operation_t operations[NUM_OPERATIONS] = 
{
	{
		0x0,
		1,
		{PARAM_TYPE_STRING, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		1,
		{PARAM_TYPE_INT, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_new_chain
	},
	{
		0x1,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STRING, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		1,
		{PARAM_TYPE_INT, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_new_state
	},
	{
		0x2,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_INT, PARAM_TYPE_INT, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_move_probs
	},
	{
		0x3,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_IP_FILTER, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_source
	},
	{
		0x4,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_IP_FILTER, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_dest
	},
	{
		0x5,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_INT, PARAM_TYPE_BOOL, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_protocol
	},
	{
		0x6,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_INT, PARAM_TYPE_BOOL, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_tos
	},
	{
		0x8,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_INT, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_length_min
	},
	{
		0x9,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_INT, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_length_max
	},
	{
		0xA,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_WHICH_PACKET_LENGTH, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_which_length
	},
	{
		0xE,
		4,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_PROB, PARAM_TYPE_PROB},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_drop_delay_prob
	},
	{
		0xF,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_DELAY_FUNCTION_TYPE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_delay_type
	},
	{
		0x10,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_SPLIT_LINEAR_POINTS, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_split_linear_points
	},
	{
		0x11,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_LAMBDA, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_lambda
	},
	{
		0x13,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_DELAY_TYPE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_time_factor
	},
	{
		0x14,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_PROB, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_stable_delay_prob
	},
	{
		0x19,
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_clear_all
	},
	{
		0x10000,
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_end_connection
	},
};


static int ip_noise_arbitrator_iface_handler_new_chain(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	char * name = params[0].string;
    int index;

    ip_noise_arbitrator_data_t * data;

    printf("New Chain: \"%s\"!\n", name);

    data = self->data;

    if (data->num_chains == data->max_num_chains)
    {
        data->max_num_chains += 16;

        data->chains = realloc(data->chains, sizeof(data->chains[0])*data->max_num_chains);        
    }

    index = data->num_chains;
    data->chains[index] = chain_alloc(name);
    
    ip_noise_str2int_dict_add(data->chain_names, name, index);
    
    self->last_chain = index;

    self->last_state = -1;
    
    data->num_chains++;

    out_params[1]._int = index;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_new_state(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	char * state_name = params[1].string;

    int a;
    int index;

    ip_noise_chain_t * chain = get_chain(self,chain_index);

    if (chain == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    printf("New state: \"%s\"!\n", state_name);
    
    for(a=0;a<chain->num_states;a++)
    {
        chain->states[a]->move_tos = realloc(chain->states[a]->move_tos, sizeof(chain->states[a]->move_tos[0])*(chain->num_states+1));
        chain->states[a]->move_tos[chain->num_states].comulative_prob = 0; 
    }

    if (chain->num_states == chain->max_num_states)
    {
        chain->max_num_states += 16;
        chain->states = realloc(
                chain->states,
                sizeof(chain->states[0])*chain->max_num_states
                );
    }

    index = chain->num_states;

    chain->states[index] = state_alloc(state_name);
    
    ip_noise_str2int_dict_add(chain->state_names, state_name, index);

    self->last_state = index;
    
    chain->states[index]->move_tos = malloc(sizeof(chain->states[index]->move_tos[0])*(index+1));
    for(a=0;a<index;a++)
    {
        chain->states[index]->move_tos[a].comulative_prob = 0;
    }
    chain->states[index]->move_tos[a].comulative_prob = 1;

    chain->num_states++;

    out_params[0].state = index;
    
	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_move_probs(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int num_sources = params[1]._int;
	int num_dests = params[2]._int;
    int i,s,d;
    int * sources;
    int * dests;
    param_t param;
    int ok;
    ip_noise_prob_t * move_to_probs;

    ip_noise_chain_t * chain;

    chain = get_chain(self, chain_index);

    if (chain == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    sources = malloc(sizeof(sources[0]) * num_sources);
    for(i=0;i<num_sources;i++)
    {
        ok = (read_param_type(self, PARAM_TYPE_STATE, &param));
        if (ok < 0)
        {
            free(sources);
            return ok;
        }                
        sources[i] = param.state;
        if (sources[i] >= chain->num_states)
        {
            free(sources);
            return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
        }
    }

    dests = malloc(sizeof(dests[0]) * num_dests);
    for(i=0;i<num_dests;i++)
    {
        ok = (read_param_type(self, PARAM_TYPE_STATE, &param));
        if (ok < 0)
        {
            free(sources);
            free(dests);
            return ok;
        }                        
        dests[i] = param.state;
        if (dests[i] >= chain->num_states)
        {
            free(sources);
            free(dests);
            return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
        }
    }

    move_to_probs = malloc(num_sources*num_dests*sizeof(ip_noise_prob_t));

    for(s=0;s<num_sources;s++)
    {
        for(d=0;d<num_dests;d++)
        {
            ok = read_param_type(self, PARAM_TYPE_PROB, &param);
            if (ok < 0)
            {
                free(move_to_probs);
                free(sources);
                free(dests);
                return ok;
            }
            move_to_probs[num_dests*s+d] = param.prob;
        }
    }

    
    /*
     * TODO: add sanity check that the sum of the dests in any source
     *  equals the present sum.
     *
     * */
    for(s=0;s<num_sources;s++)
    {
        for(d=0;d<num_dests;d++)
        {
            chain->states[sources[s]]->move_tos[dests[d]].comulative_prob = move_to_probs[num_dests*s+d];
        }
    }

    /*
     * I think I'm getting too used to garbage collection... That's Good!
     * */
    free(sources);
    free(dests);
    free(move_to_probs);

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_source(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	ip_noise_ip_spec_t * source = params[1].ip_filter;
    
    ip_noise_chain_t * chain;

    printf("Set Source!\n");

    chain = get_chain(self, chain_index);

    if (chain == NULL)
    {
        ip_spec_free(source);
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    chain->filter->source = source;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_dest(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	ip_noise_ip_spec_t * dest = params[1].ip_filter;

    ip_noise_chain_t * chain;

    printf("Set Dest!\n");

    chain = get_chain(self, chain_index);

    if (chain == NULL)
    {
        ip_spec_free(dest);
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    chain->filter->dest = dest;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_protocol(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int index = params[1]._int;
	int enable_or_disable = params[2].bool;

    ip_noise_chain_t * chain;
    int byte, bit;

    printf("Set Protocol! - (%i,%i)\n", index, enable_or_disable);
    
    chain = get_chain(self, chain_index);

    if (chain == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    byte = (index >> 3);
    bit = (index & 0x7);

    if ((byte < 0) || (byte >= sizeof(chain->filter->protocols)))
    {
        memset(
            chain->filter->protocols ,
            (enable_or_disable?'\xFF':'\x00') ,
            sizeof(chain->filter->protocols)
            );
        
    }
    else
    {
        if (enable_or_disable)
        {
            chain->filter->protocols[byte] |= (1<<bit);
        }
        else
        {
            chain->filter->protocols[byte] &= (~(1<<bit));
        }
    }

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_tos(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int index = params[1]._int;
	int enable_or_disable = params[2].bool;

    ip_noise_chain_t * chain;
    int byte, bit;

    printf("Set TOS! - (%i,%i)\n", index, enable_or_disable);
    
    chain = get_chain(self, chain_index);

    if (chain == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    byte = (index >> 3);
    bit = (index & 0x7);

    if ((byte < 0) || (byte >= sizeof(chain->filter->tos)))
    {
        memset(
            chain->filter->tos ,
            (enable_or_disable?'\xFF':'\x00') ,
            sizeof(chain->filter->tos)
            );
        
    }
    else
    {
        if (enable_or_disable)
        {
            chain->filter->tos[byte] |= (1<<bit);
        }
        else
        {
            chain->filter->tos[byte] &= (~(1<<bit));
        }
    }

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_length_min(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int min_ = params[1]._int;

    ip_noise_chain_t * chain;

    printf("Set Min Length!\n");
    
    chain = get_chain(self, chain_index);

    if (chain == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    chain->filter->min_packet_len = min_;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_length_max(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int max_ = params[1]._int;

    ip_noise_chain_t * chain;

    printf("Set Max Length!\n");
    
    chain = get_chain(self, chain_index);

    if (chain == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    chain->filter->max_packet_len = max_;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_which_length(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int which = params[1].which_packet_length;

    ip_noise_chain_t * chain;

    printf("Set Which Length!\n");
    
    chain = get_chain(self, chain_index);

    if (chain == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    chain->filter->which_packet_len = which;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_drop_delay_prob(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	ip_noise_prob_t drop_prob = params[2].prob;
	ip_noise_prob_t delay_prob = params[3].prob;

    ip_noise_state_t * state;

    printf("Set Drop/Delay Probs - (%i,%i)!\n", chain_index, state_index);

    state = get_state(self, chain_index, state_index);

    if (state == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    state->drop_prob = drop_prob;
    state->delay_prob = delay_prob;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_delay_type(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	int delay_type = params[2].delay_function_type;

    ip_noise_state_t * state;

    printf("Set Delay Type!\n");

    state = get_state(self, chain_index, state_index);

    if (state == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    /* Free the split linear points in case the type is no longer split-linear */
    if ((state->delay_function.type == IP_NOISE_DELAY_FUNCTION_SPLIT_LINEAR) &&
        (delay_type != IP_NOISE_DELAY_FUNCTION_SPLIT_LINEAR))
    {
        free(state->delay_function.params.split_linear.points);
    }
    state->delay_function.type = delay_type;
    if (delay_type == IP_NOISE_DELAY_FUNCTION_SPLIT_LINEAR)
    {
        state->delay_function.params.split_linear.num_points = 0;
        state->delay_function.params.split_linear.points = NULL;        
    }
    
	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_split_linear_points(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	ip_noise_split_linear_function_t points = params[2].split_linear_points;

    ip_noise_state_t * state;

    printf("Set Split Linear Points!\n");

    state = get_state(self, chain_index, state_index);

    if (state == NULL)
    {
        free(points.points);
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    if (state->delay_function.type != IP_NOISE_DELAY_FUNCTION_SPLIT_LINEAR)
    {
        free(points.points);
        return IP_NOISE_RET_VALUE_SPLIT_LINEAR_SET_POINTS_ON_OTHER_TYPE;
    }

    if(state->delay_function.params.split_linear.points != NULL)
    {
        free(state->delay_function.params.split_linear.points);
    }

    state->delay_function.params.split_linear = points;
    
	return 0;
}


static int ip_noise_arbitrator_iface_handler_set_lambda(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	int lambda = params[2].lambda;

    ip_noise_state_t * state;

    printf("Set Lambda!\n");

    state = get_state(self, chain_index, state_index);

    if (state == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    if (state->delay_function.type != IP_NOISE_DELAY_FUNCTION_EXP)
    {
        return IP_NOISE_RET_VALUE_SPLIT_LINEAR_SET_POINTS_ON_OTHER_TYPE;
    }

    state->delay_function.params.lambda = lambda;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_time_factor(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	int time_factor = params[2].delay_type;

    ip_noise_state_t * state;

    printf("Set Time Factor = %i\n", time_factor);

    state = get_state(self, chain_index, state_index);

    if (state == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }
    
    state->time_factor = time_factor;
    
	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_stable_delay_prob(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	ip_noise_prob_t stable_delay_prob = params[2].prob;

    ip_noise_state_t * state;



    printf("Set Stable Delay Prob - %f!\n", stable_delay_prob);

    state = get_state(self, chain_index, state_index);

    if (state == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    state->stable_delay_prob = stable_delay_prob;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_clear_all(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
    ip_noise_arbitrator_data_t * data = self->data;

    int a;

    printf("Clear All!\n");

    
    for(a=0;a<data->num_chains;a++)
    {
        chain_free(data->chains[a]);
    }

    data->num_chains = 0;

    ip_noise_str2int_dict_reset(data->chain_names);

	return 0;
}



static int ip_noise_arbitrator_iface_handler_end_connection(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
    self->_continue = 0;

	return 0;
}



